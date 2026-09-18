#include "bassamp/Amp.h"

namespace bassamp {

const char* ampName (AmpType t)
{
    switch (t)
    {
        case AmpType::AshdownMag:    return "MAG 500 (solid state head)";
        case AmpType::AmpegSvt:      return "SVT (300W all-valve)";
        case AmpType::AmpegB15:      return "B-15 Flip-Top (portable valve)";
        case AmpType::FenderBassman: return "Tweed Bassman (5F6-A)";
        case AmpType::StudioDI:      return "Studio DI (transformer)";
        case AmpType::DriverDI:      return "Driver DI (bass preamp pedal)";
        default: return "?";
    }
}

const char* ampDescription (AmpType t)
{
    switch (t)
    {
        case AmpType::AshdownMag:
            return "FET preamp, stiff supply, lots of clean headroom. Four-band EQ with a Shape "
                   "switch for the classic scooped curve, plus a sub-harmoniser. Stays clean where "
                   "a valve amp would already be compressing.";
        case AmpType::AmpegSvt:
            return "The rock bass standard. Two valve gain stages into a big push-pull output "
                   "section: firm lows, forward midrange, and it thickens rather than fizzes when "
                   "you push it. Selectable midrange plus Ultra Lo / Ultra Hi voicing switches.";
        case AmpType::AmpegB15:
            return "Low-powered flip-top. Very little headroom, so it compresses and breaks up "
                   "early - the Motown and studio sound. Simple Bass/Treble tone, heavy supply sag.";
        case AmpType::FenderBassman:
            return "Tweed-era 5F6-A. Interactive passive tone stack with the famous mid scoop that "
                   "deepens as you back the mid control off. Warm, loose bottom, plenty of grind.";
        case AmpType::StudioDI:
            return "Transformer-coupled studio DI. Essentially clean: gentle three-band shaping and "
                   "a little iron on the transients. The reference for a recorded DI track.";
        case AmpType::DriverDI:
            return "Bass driver preamp/DI. Blend control keeps the clean low end intact while the "
                   "driven path adds grind, with a built-in speaker-emulation rolloff.";
        default: return "";
    }
}

bool ampHasMidFreq  (AmpType t) { return t == AmpType::AshdownMag || t == AmpType::AmpegSvt || t == AmpType::StudioDI; }
bool ampHasPresence (AmpType t) { return t != AmpType::AmpegB15 && t != AmpType::StudioDI; }
bool ampHasUltra    (AmpType t) { return t == AmpType::AmpegSvt; }
bool ampHasShape    (AmpType t) { return t == AmpType::AshdownMag; }
bool ampHasSub      (AmpType t) { return t == AmpType::AshdownMag; }
bool ampHasBlend    (AmpType t) { return t == AmpType::DriverDI; }
bool ampHasBright   (AmpType t) { return t == AmpType::FenderBassman || t == AmpType::AmpegB15 || t == AmpType::AshdownMag; }

int ampMidFreqCount (AmpType t) { return ampHasMidFreq (t) ? 5 : 1; }

float ampMidFreqValue (AmpType t, int index)
{
    index = std::max (0, std::min (index, 4));
    switch (t)
    {
        case AmpType::AmpegSvt:   { static const float f[] = { 220.0f, 450.0f, 800.0f, 1600.0f, 3000.0f }; return f[index]; }
        case AmpType::AshdownMag: { static const float f[] = { 180.0f, 350.0f, 700.0f, 1400.0f, 2800.0f }; return f[index]; }
        case AmpType::StudioDI:   { static const float f[] = { 120.0f, 250.0f, 500.0f, 1000.0f, 2000.0f }; return f[index]; }
        default: return 700.0f;
    }
}

void Amp::prepare (double sampleRate, int maxBlockSize, int oversampleFactor)
{
    baseRate = sampleRate;
    maxBlock = std::max (1, maxBlockSize);
    osFactor = (oversampleFactor >= 4 ? 4 : (oversampleFactor >= 2 ? 2 : 1));
    osRate   = baseRate * osFactor;

    oversampler.prepare (baseRate, maxBlock, osFactor);
    dryBuffer.assign ((size_t) maxBlock, 0.0f);

    inputHp.setHighpass (baseRate, 26.0f, 0.707f);
    outputLp.setLowpass (baseRate, std::min (18000.0f, (float) baseRate * 0.45f), 0.707f);
    postDc.prepare (baseRate);

    triode1.prepare (osRate);
    triode2.prepare (osRate);
    fet1.prepare (osRate);
    fet2.prepare (osRate);
    power.prepare (osRate);

    sub.prepare (baseRate);
    subLp.setLowpass (baseRate, 180.0f, 0.707f);

    gainSmooth.reset   (osRate,   0.020f, 1.0f);
    masterSmooth.reset (osRate,   0.020f, 1.0f);
    outputSmooth.reset (baseRate, 0.020f, 1.0f);
    blendSmooth.reset  (baseRate, 0.020f, 1.0f);
    subSmooth.reset    (baseRate, 0.020f, 0.0f);

    driveRelease = timeConstantCoeff (0.25f, baseRate);

    updateVoicing();
    reset();
}

void Amp::setOversampling (int factor)
{
    if (factor == osFactor) return;
    prepare (baseRate, maxBlock, factor);
}

void Amp::reset()
{
    inputHp.reset(); outputLp.reset(); postDc.reset();
    for (auto& b : tone) b.reset();
    brightShelf.reset(); presenceShelf.reset();
    triode1.reset(); triode2.reset(); fet1.reset(); fet2.reset();
    power.reset(); sub.reset(); subLp.reset();
    oversampler.reset();
    driveMeter = 0.0f;
}

void Amp::setSettings (const AmpSettings& s)
{
    const bool voicingChanged =
        s.type != settings.type || s.bass != settings.bass || s.mid != settings.mid
        || s.treble != settings.treble || s.midFreqIndex != settings.midFreqIndex
        || s.presence != settings.presence || s.bright != settings.bright
        || s.ultraLo != settings.ultraLo || s.ultraHi != settings.ultraHi
        || s.shape != settings.shape || s.gain != settings.gain || s.sag != settings.sag;

    const bool typeChanged = s.type != settings.type;
    settings = s;
    if (voicingChanged) updateVoicing();
    if (typeChanged) reset();
}

void Amp::updateVoicing()
{
    toneCount = 0;
    auto addPeak = [this] (float f, float q, float g)
    {
        if (std::fabs (g) < 0.05f || toneCount >= kToneSections) return;
        tone[toneCount++].setPeaking (osRate, f, q, g);
    };
    auto addLowShelf = [this] (float f, float q, float g)
    {
        if (std::fabs (g) < 0.05f || toneCount >= kToneSections) return;
        tone[toneCount++].setLowShelf (osRate, f, q, g);
    };
    auto addHighShelf = [this] (float f, float q, float g)
    {
        if (std::fabs (g) < 0.05f || toneCount >= kToneSections) return;
        tone[toneCount++].setHighShelf (osRate, f, q, g);
    };
    auto addLowPass = [this] (float f, float q)
    {
        if (toneCount >= kToneSections) return;
        tone[toneCount++].setLowpass (osRate, f, q);
    };
    auto addHighPass = [this] (float f, float q)
    {
        if (toneCount >= kToneSections) return;
        tone[toneCount++].setHighpass (osRate, f, q);
    };

    const float b = clampf (settings.bass,   0.0f, 1.0f);
    const float m = clampf (settings.mid,    0.0f, 1.0f);
    const float t = clampf (settings.treble, 0.0f, 1.0f);
    const float midF = ampMidFreqValue (settings.type, settings.midFreqIndex);

    makeupGain = 1.0f;
    preGainTarget = 1.0f;
    brightActive = false;
    presenceActive = false;

    switch (settings.type)
    {
        case AmpType::AshdownMag:
            addLowShelf  (65.0f,   0.70f, (b - 0.5f) * 24.0f);
            addPeak      (midF,    0.90f, (m - 0.5f) * 24.0f);
            addHighShelf (5000.0f, 0.70f, (t - 0.5f) * 24.0f);
            if (settings.shape)
            {
                // The Shape switch is a fixed loudness curve: lows and highs up,
                // 500 Hz out. Great for slap, wrong for sitting in a mix.
                addLowShelf  (80.0f,   0.70f,  5.0f);
                addPeak      (500.0f,  0.80f, -8.0f);
                addHighShelf (4000.0f, 0.70f,  5.0f);
            }
            preGainTarget = 1.0f + 9.0f * settings.gain;
            fet1.gain = 1.0f;
            fet1.knee = 0.95f;
            fet2.gain = 0.85f;
            fet2.knee = 1.10f;
            power.headroom  = 1.70f;    // big solid-state rail
            power.sagAmount = 0.10f * settings.sag;
            power.ironSaturation = 0.15f;
            break;

        case AmpType::AmpegSvt:
            addLowShelf  (40.0f,   0.60f, (b - 0.5f) * 26.0f);
            addPeak      (midF,    0.80f, (m - 0.5f) * 26.0f);
            addHighShelf (4000.0f, 0.70f, (t - 0.5f) * 24.0f);
            addPeak      (700.0f,  0.80f, 1.5f);          // the SVT's own midrange push
            if (settings.ultraLo)
            {
                addLowShelf  (60.0f,   0.70f,  4.0f);
                addPeak      (500.0f,  0.80f, -6.0f);
                addHighShelf (8000.0f, 0.70f,  2.0f);
            }
            if (settings.ultraHi) addPeak (6000.0f, 1.00f, 8.0f);
            preGainTarget = 1.0f + 11.0f * settings.gain;
            triode1.gain = 1.0f;
            triode1.posHeadroom = 0.80f; triode1.negHeadroom = 1.40f; triode1.biasShift = 0.30f;
            triode2.gain = 1.25f;
            triode2.posHeadroom = 0.90f; triode2.negHeadroom = 1.30f; triode2.biasShift = 0.22f;
            power.headroom  = 1.25f;
            power.sagAmount = 0.35f * settings.sag + 0.10f;
            power.ironSaturation = 0.50f;
            break;

        case AmpType::AmpegB15:
            addLowShelf  (100.0f,  0.70f, (b - 0.5f) * 20.0f);
            addPeak      (700.0f,  0.80f, (m - 0.5f) * 12.0f);   // not on the original: a gentle extra
            addHighShelf (3000.0f, 0.70f, (t - 0.5f) * 20.0f);
            addPeak      (120.0f,  0.80f, 2.0f);
            addLowPass   (6500.0f, 0.70f);
            preGainTarget = 1.0f + 15.0f * settings.gain;
            triode1.gain = 1.0f;
            triode1.posHeadroom = 0.62f; triode1.negHeadroom = 1.15f; triode1.biasShift = 0.50f;
            triode2.gain = 1.60f;
            triode2.posHeadroom = 0.70f; triode2.negHeadroom = 1.05f; triode2.biasShift = 0.40f;
            power.headroom  = 0.72f;    // very little of it, which is the whole point
            power.sagAmount = 0.75f * settings.sag + 0.20f;
            power.ironSaturation = 0.85f;
            break;

        case AmpType::FenderBassman:
        {
            // Passive FMV stack: the scoop sits between the bass and treble
            // controls, it deepens as the mid control comes down, and its centre
            // moves up as you add treble. That interaction is the sound.
            const float notch = 380.0f + 220.0f * t;
            addPeak      (notch,   0.90f, -9.0f + 15.0f * m);
            addLowShelf  (110.0f,  0.70f, -14.0f + 22.0f * b);
            addHighShelf (2200.0f, 0.60f, -14.0f + 24.0f * t);
            makeupGain = dbToGain (7.0f);     // the stack's insertion loss, made back up
            preGainTarget = 1.0f + 13.0f * settings.gain;
            triode1.gain = 1.0f;
            triode1.posHeadroom = 0.70f; triode1.negHeadroom = 1.30f; triode1.biasShift = 0.38f;
            triode2.gain = 1.35f;
            triode2.posHeadroom = 0.78f; triode2.negHeadroom = 1.20f; triode2.biasShift = 0.30f;
            power.headroom  = 0.90f;
            power.sagAmount = 0.55f * settings.sag + 0.15f;
            power.ironSaturation = 0.70f;
            break;
        }

        case AmpType::StudioDI:
            addHighPass  (22.0f,   0.70f);
            addLowShelf  (80.0f,   0.70f, (b - 0.5f) * 20.0f);
            addPeak      (midF,    0.80f, (m - 0.5f) * 16.0f);
            addHighShelf (6000.0f, 0.70f, (t - 0.5f) * 20.0f);
            preGainTarget = 1.0f + 1.5f * settings.gain;
            power.headroom  = 4.00f;    // effectively clean
            power.sagAmount = 0.0f;
            power.ironSaturation = 0.18f;
            break;

        case AmpType::DriverDI:
            addLowShelf  (80.0f,   0.70f, (b - 0.5f) * 24.0f);
            addPeak      (1000.0f, 0.70f, (m - 0.5f) * 16.0f);
            addHighShelf (4000.0f, 0.70f, (t - 0.5f) * 24.0f);
            // Built-in speaker emulation - the reason these pedals work straight
            // into a desk.
            addHighPass  (65.0f,   0.80f);
            addPeak      (120.0f,  0.90f,  3.0f);
            addPeak      (800.0f,  1.00f, -3.0f);
            addLowPass   (3800.0f, 0.80f);
            preGainTarget = 1.0f + 13.0f * settings.gain;
            fet1.gain = 1.0f;
            fet1.knee = 0.80f; fet1.asymmetry = 1.18f;
            fet2.gain = 1.0f;
            fet2.knee = 0.95f;
            power.headroom  = 3.00f;
            power.sagAmount = 0.0f;
            power.ironSaturation = 0.10f;
            break;

        default: break;
    }

    if (ampHasBright (settings.type) && settings.bright)
    {
        brightShelf.setHighShelf (osRate, 1500.0f, 0.70f, 6.0f);
        brightActive = true;
    }
    if (ampHasPresence (settings.type) && settings.presence > 0.01f)
    {
        // Presence on a valve amp works inside the negative feedback loop, so it
        // lifts the top end and reduces damping with it.
        presenceShelf.setHighShelf (osRate, 3500.0f, 0.70f, settings.presence * 9.0f);
        presenceActive = true;
    }

    // Re-prepare the nonlinear stages so the new voicing constants take effect.
    triode1.prepare (osRate);
    triode2.prepare (osRate);
    fet1.prepare (osRate);
    fet2.prepare (osRate);
    power.prepare (osRate);
}

void Amp::process (float* data, int numSamples)
{
    if (numSamples <= 0) return;
    if ((int) dryBuffer.size() < numSamples) dryBuffer.assign ((size_t) numSamples, 0.0f);

    const AmpType type = settings.type;
    const bool usesTriodes = (type == AmpType::AmpegSvt || type == AmpType::AmpegB15 || type == AmpType::FenderBassman);
    const bool usesFets    = (type == AmpType::AshdownMag || type == AmpType::DriverDI);
    const bool hasSecondStage = (type != AmpType::StudioDI);

    // Keep a dry copy for the DI blend control.
    std::copy (data, data + numSamples, dryBuffer.begin());

    // Input conditioning and the sub-harmoniser, both at base rate.
    const float subTarget = ampHasSub (type) ? settings.subLevel : 0.0f;
    subSmooth.setTarget (subTarget);

    for (int n = 0; n < numSamples; ++n)
    {
        float x = inputHp.process (data[n]);
        const float subAmt = subSmooth.next();
        if (subAmt > 0.0001f)
        {
            const auto oct = sub.process (x);
            x += subLp.process (oct.down1) * subAmt * 1.4f;
        }
        data[n] = x;

        const float a = std::fabs (x);
        driveMeter = (a > driveMeter) ? a : driveMeter * driveRelease;
    }

    // --- oversampled nonlinear section --------------------------------------
    gainSmooth.setTarget (preGainTarget);
    masterSmooth.setTarget (0.4f + 2.6f * clampf (settings.master, 0.0f, 1.0f));

    float* os = oversampler.upsample (data, numSamples);
    const int osN = numSamples * oversampler.getFactor();

    for (int i = 0; i < osN; ++i)
    {
        float x = os[i];

        if (brightActive) x = brightShelf.process (x);

        // Preamp drive is applied as a smoothed multiplier rather than baked into
        // the stage constants, so turning the gain knob while playing does not
        // step the coefficient and click.
        x *= gainSmooth.next();

        if (usesTriodes)      x = triode1.process (x);
        else if (usesFets)    x = fet1.process (x);

        for (int s = 0; s < toneCount; ++s) x = tone[s].process (x);
        x *= makeupGain;

        if (hasSecondStage)
        {
            if (usesTriodes)   x = triode2.process (x);
            else if (usesFets) x = fet2.process (x);
        }

        if (presenceActive) x = presenceShelf.process (x);

        x = power.process (x * masterSmooth.next());

        os[i] = x;
    }

    oversampler.downsample (data, numSamples);

    // --- base rate output ---------------------------------------------------
    outputSmooth.setTarget (dbToGain (-24.0f + 30.0f * clampf (settings.output, 0.0f, 1.0f)));
    blendSmooth.setTarget (ampHasBlend (type) ? clampf (settings.blend, 0.0f, 1.0f) : 1.0f);

    for (int n = 0; n < numSamples; ++n)
    {
        const float wet = data[n];
        const float bl  = blendSmooth.next();
        float y = wet * bl + dryBuffer[(size_t) n] * (1.0f - bl);
        y = postDc.process (outputLp.process (y));
        data[n] = y * outputSmooth.next();
    }
}

float Amp::toneMagnitudeDbAt (float freq) const
{
    float mag = 1.0f;
    for (int s = 0; s < toneCount; ++s) mag *= tone[s].magnitudeAt (freq, osRate);
    if (brightActive)   mag *= brightShelf.magnitudeAt (freq, osRate);
    if (presenceActive) mag *= presenceShelf.magnitudeAt (freq, osRate);
    return gainToDb (mag * makeupGain);
}

} // namespace bassamp
