// Fuzz, distortion, overdrive, synth and the digital destruction boxes.
#include "bassamp/Pedals.h"

namespace bassamp {

// ============================================================================
//  Bass Synth
// ============================================================================

static const char* const kSynthWaves[] = { "Square", "Saw", "Pulse", "Ramp+Sub" };

const ParamDesc SynthPedal::descs[] =
{
    { "Sensitivity", 0.0f, 100.0f,  55.0f, "%",  0, nullptr, 0.5f },
    { "Filter",    120.0f, 4000.0f, 600.0f, "Hz", 0, nullptr, 0.3f },
    { "Resonance",   0.5f,  12.0f,   4.5f, "Q",  0, nullptr, 0.4f },
    { "Sweep",    -100.0f, 100.0f,  65.0f, "%",  0, nullptr, 0.5f },
    { "Decay",      30.0f, 800.0f, 180.0f, "ms", 0, nullptr, 0.35f },
    { "Wave",        0.0f,   3.0f,   0.0f, "",   4, kSynthWaves, 0.5f },
    { "Sub",         0.0f, 100.0f,  35.0f, "%",  0, nullptr, 0.5f },
    { "Dry Mix",     0.0f, 100.0f,  30.0f, "%",  0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (SynthPedal)

void SynthPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    trackLp1.setLowpass (sampleRate, 300.0f, 0.707f);
    trackLp2.setLowpass (sampleRate, 300.0f, 0.707f);
    filter.prepare (sampleRate);
    amplitude.prepare (sampleRate);
    sweepEnv.prepare (sampleRate);
    divider.prepare (sampleRate);
    outputDc.prepare (sampleRate);
    restoreDefaults();
    reset();
}

void SynthPedal::reset()
{
    trackLp1.reset(); trackLp2.reset(); filter.reset();
    amplitude.reset(); sweepEnv.reset(); divider.reset(); outputDc.reset();
    oscPhase = 0.0f; oscInc = 0.0f;
    periodEstimate = (float) fs / 110.0f;
    samplesSinceCross = 0;
    above = false;
}

void SynthPedal::paramsChanged()
{
    amplitude.setTimes (0.002f, 0.030f);
    sweepEnv.setTimes (0.004f, paramAt (4) * 0.001f);
}

void SynthPedal::process (float* data, int numSamples)
{
    const float sensitivity = 0.2f + paramAt (0) * 0.06f;
    const float baseCutoff  = paramAt (1);
    const float resonance   = paramAt (2);
    const float sweep       = paramAt (3) * 0.01f;
    const int   wave        = choiceAt (5);
    const float subLevel    = paramAt (6) * 0.01f;
    const float dryMix      = paramAt (7) * 0.01f;

    for (int n = 0; n < numSamples; ++n)
    {
        const float x = data[n];
        const float tracked = trackLp2.process (trackLp1.process (x));
        const float amp = amplitude.process (x);

        // Cycle detection on the band-limited fundamental. Measuring the period
        // between rising zero crossings and driving a phase accumulator from it
        // gives an oscillator that is locked in pitch AND in phase - no detection
        // window, so no latency and no octave jumps on the attack.
        trigHysteresis = std::max (0.003f, amp * 0.06f);
        ++samplesSinceCross;

        const bool nowAbove = above ? (tracked > -trigHysteresis) : (tracked > trigHysteresis);
        if (nowAbove && ! above)
        {
            const float measured = (float) samplesSinceCross;
            // Accept 30 Hz .. 800 Hz; anything outside is a mis-trigger.
            if (measured > fs / 800.0f && measured < fs / 30.0f)
            {
                // Light smoothing: follow a real pitch change quickly, ignore jitter.
                periodEstimate = 0.35f * periodEstimate + 0.65f * measured;
                oscPhase = 0.0f;
            }
            samplesSinceCross = 0;
        }
        above = nowAbove;

        oscInc = 1.0f / std::max (8.0f, periodEstimate);
        oscPhase += oscInc;
        if (oscPhase >= 1.0f) oscPhase -= 1.0f;

        float osc = 0.0f;
        switch (wave)
        {
            case 0: osc = oscPhase < 0.5f ? 1.0f : -1.0f; break;                    // square
            case 1: osc = 1.0f - 2.0f * oscPhase; break;                            // saw
            case 2: osc = oscPhase < 0.25f ? 1.0f : -1.0f; break;                   // narrow pulse
            default: osc = (1.0f - 2.0f * oscPhase) * 0.7f
                          + (oscPhase < 0.5f ? 0.3f : -0.3f); break;                // ramp + square
        }
        osc *= amp * 2.2f;

        if (subLevel > 0.001f)
        {
            const auto o = divider.process (x);
            osc += o.down1 * subLevel * 2.0f;
        }

        // Envelope-swept resonant filter. Negative sweep runs the filter down
        // instead of up, which is the "reverse" setting on the classic pedals.
        const float env = sweepEnv.process (x) * sensitivity;
        const float cutoff = clampf (baseCutoff * std::pow (2.0f, sweep * 4.0f * clampf (env, 0.0f, 1.5f)),
                                     40.0f, (float) fs * 0.45f);
        filter.setParams (cutoff, resonance);
        const float filtered = filter.process (osc).lp;

        data[n] = outputDc.process (filtered) + x * dryMix;
    }
}

// ============================================================================
//  Fuzz
// ============================================================================

const ParamDesc FuzzPedal::descs[] =
{
    { "Sustain",   0.0f, 100.0f,  70.0f, "%",  0, nullptr, 0.5f },
    { "Tone",      0.0f, 100.0f,  45.0f, "%",  0, nullptr, 0.5f },
    { "Level",   -24.0f,  12.0f,  -6.0f, "dB", 0, nullptr, 0.5f },
    { "Low Keep", 40.0f, 400.0f, 130.0f, "Hz", 0, nullptr, 0.4f },
    { "Blend",     0.0f, 100.0f, 100.0f, "%",  0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (FuzzPedal)

void FuzzPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    split.prepare (sampleRate);
    dc1.prepare (sampleRate);
    dc2.prepare (sampleRate);
    restoreDefaults();
    reset();
}

void FuzzPedal::reset()
{
    split.reset();
    stage1Hp.reset(); stage2Hp.reset(); toneLp.reset(); toneHp.reset(); postLp.reset();
    dc1.reset(); dc2.reset();
}

void FuzzPedal::paramsChanged()
{
    split.setFrequency (paramAt (3));
    // Each clipping stage is preceded by a high-pass. Thinning before the diodes
    // and putting the weight back afterwards is why this circuit stays defined
    // instead of turning to mush.
    stage1Hp.setHighpass (fs, 300.0f, 0.707f);
    stage2Hp.setHighpass (fs, 420.0f, 0.707f);
    toneLp.setLowpass (fs, 900.0f, 0.707f);
    toneHp.setHighpass (fs, 900.0f, 0.707f);
    postLp.setLowpass (fs, 6500.0f, 0.707f);
}

void FuzzPedal::process (float* data, int numSamples)
{
    const float sustain = 2.0f + paramAt (0) * 0.9f;    // up to ~90x into the first stage
    const float tone    = paramAt (1) * 0.01f;
    const float level   = dbToGain (paramAt (2));
    const float blend   = paramAt (4) * 0.01f;

    for (int n = 0; n < numSamples; ++n)
    {
        const float dry = data[n];

        float low, high;
        split.crossover.process (dry, low, high);

        // Stage one: soft, high gain.
        float y = stage1Hp.process (high) * sustain;
        y = dc1.process (fastTanh (y));

        // Stage two: harder, and it is the asymmetry here that gives the fuzz its
        // buzz rather than a clean square.
        y = stage2Hp.process (y) * 6.0f;
        y = dc2.process (y >= 0.0f ? fastTanh (y * 1.25f) : fastTanh (y) * 0.82f);

        // Tone network: crossfade a low-passed and a high-passed copy. With the
        // control at noon both are present and the midrange between them drops
        // out, which is the scoop these circuits are known for.
        const float toned = toneLp.process (y) * (1.0f - tone) + toneHp.process (y) * tone;
        const float fuzz = postLp.process (toned) * level;

        data[n] = low + (fuzz * blend + high * (1.0f - blend));
    }
}

// ============================================================================
//  Distortion
// ============================================================================

const ParamDesc DistortionPedal::descs[] =
{
    { "Drive",     0.0f, 100.0f,  55.0f, "%",  0, nullptr, 0.5f },
    { "Tone",    -12.0f,  12.0f,   2.0f, "dB", 0, nullptr, 0.5f },
    { "Level",   -24.0f,  12.0f,  -6.0f, "dB", 0, nullptr, 0.5f },
    { "Low Keep", 40.0f, 400.0f, 100.0f, "Hz", 0, nullptr, 0.4f },
    { "Mix",       0.0f, 100.0f, 100.0f, "%",  0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (DistortionPedal)

void DistortionPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    split.prepare (sampleRate);
    dc.prepare (sampleRate);
    restoreDefaults();
    reset();
}

void DistortionPedal::reset()
{
    split.reset(); preEmphasis.reset(); toneTilt.reset(); postLp.reset(); dc.reset();
}

void DistortionPedal::paramsChanged()
{
    split.setFrequency (paramAt (3));
    preEmphasis.setPeaking (fs, 1200.0f, 0.8f, 6.0f);   // what gets clipped hardest
    toneTilt.setHighShelf (fs, 2000.0f, 0.7f, paramAt (1));
    postLp.setLowpass (fs, 5000.0f, 0.707f);
}

void DistortionPedal::process (float* data, int numSamples)
{
    const float drive = 1.0f + paramAt (0) * 0.6f;
    const float level = dbToGain (paramAt (2));
    const float mix   = paramAt (4) * 0.01f;

    for (int n = 0; n < numSamples; ++n)
    {
        const float dry = data[n];
        float low, high;
        split.crossover.process (dry, low, high);

        float y = preEmphasis.process (high) * drive;

        // Diode clipper: soft until the forward voltage, then very firm.
        const float threshold = 0.32f;
        const float a = std::fabs (y);
        float clipped;
        if (a < threshold) clipped = y;
        else clipped = (y > 0.0f ? 1.0f : -1.0f) * (threshold + (1.0f - threshold) * fastTanh ((a - threshold) / (1.0f - threshold)));
        // Slight asymmetry - a real pedal never has matched diodes.
        if (clipped < 0.0f) clipped *= 0.88f;

        y = postLp.process (toneTilt.process (dc.process (clipped))) * level;
        data[n] = low + (y * mix + high * (1.0f - mix));
    }
}

// ============================================================================
//  Overdrive
// ============================================================================

const ParamDesc OverdrivePedal::descs[] =
{
    { "Drive", 0.0f, 100.0f, 45.0f, "%",  0, nullptr, 0.5f },
    { "Tone",  0.0f, 100.0f, 50.0f, "%",  0, nullptr, 0.5f },
    { "Level", -24.0f, 12.0f, -3.0f, "dB", 0, nullptr, 0.5f },
    { "Mix",   0.0f, 100.0f, 100.0f, "%", 0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (OverdrivePedal)

void OverdrivePedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    dc.prepare (sampleRate);
    restoreDefaults();
    reset();
}

void OverdrivePedal::reset() { loopHp.reset(); midHump.reset(); toneLp.reset(); dc.reset(); }

void OverdrivePedal::paramsChanged()
{
    // The high-pass inside the feedback loop is the whole trick: only what is
    // above it gets the gain, so the fundamental passes through clean.
    loopHp.setHighpass (fs, 720.0f, 0.707f);
    midHump.setPeaking (fs, 720.0f, 0.9f, 4.0f);
    toneLp.setLowpass (fs, logRange (paramAt (1) * 0.01f, 900.0f, 7000.0f), 0.707f);
}

void OverdrivePedal::process (float* data, int numSamples)
{
    const float drive = 1.0f + paramAt (0) * 0.35f;
    const float level = dbToGain (paramAt (2));
    const float mix   = paramAt (3) * 0.01f;

    for (int n = 0; n < numSamples; ++n)
    {
        const float dry = data[n];

        const float boosted = loopHp.process (dry) * drive;
        const float clipped = fastTanh (boosted);
        // The clean path around the clipping stage keeps the low end intact.
        float y = clipped + dry * 0.35f;
        y = toneLp.process (midHump.process (dc.process (y))) * level;

        data[n] = y * mix + dry * (1.0f - mix);
    }
}

// ============================================================================
//  Bit crusher
// ============================================================================

const ParamDesc BitCrusherPedal::descs[] =
{
    { "Bits",       1.0f, 16.0f,  8.0f, "bit", 0, nullptr, 0.5f },
    { "Downsample", 1.0f, 50.0f,  4.0f, "x",   0, nullptr, 0.35f },
    { "Mix",        0.0f, 100.0f, 60.0f, "%",  0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (BitCrusherPedal)

void BitCrusherPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    restoreDefaults();
    reset();
}

void BitCrusherPedal::reset() { held = 0.0f; phase = 0.0f; }

void BitCrusherPedal::process (float* data, int numSamples)
{
    const float levels = std::pow (2.0f, paramAt (0)) - 1.0f;
    const float step   = paramAt (1);
    const float mix    = paramAt (2) * 0.01f;

    for (int n = 0; n < numSamples; ++n)
    {
        const float dry = data[n];
        phase += 1.0f;
        if (phase >= step)
        {
            phase -= step;
            held = std::round (clampf (dry, -1.0f, 1.0f) * levels) / levels;
        }
        data[n] = held * mix + dry * (1.0f - mix);
    }
}

// ============================================================================
//  Ring modulator
// ============================================================================

static const char* const kRingShapes[] = { "Sine", "Triangle", "Square" };

const ParamDesc RingModPedal::descs[] =
{
    { "Frequency", 5.0f, 2000.0f, 120.0f, "Hz", 0, nullptr, 0.25f },
    { "Shape",     0.0f,    2.0f,   0.0f, "",   3, kRingShapes, 0.5f },
    { "Mix",       0.0f,  100.0f,  40.0f, "%",  0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (RingModPedal)

void RingModPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    carrier.prepare (sampleRate);
    restoreDefaults();
    reset();
}

void RingModPedal::reset() { carrier.reset(); }

void RingModPedal::process (float* data, int numSamples)
{
    carrier.setRate (paramAt (0));
    const int shapeIndex = choiceAt (1);
    const Lfo::Shape shape = shapeIndex == 0 ? Lfo::Shape::Sine
                           : shapeIndex == 1 ? Lfo::Shape::Triangle : Lfo::Shape::Square;
    const float mix = paramAt (2) * 0.01f;

    for (int n = 0; n < numSamples; ++n)
    {
        const float dry = data[n];
        data[n] = dry * carrier.next (shape) * mix + dry * (1.0f - mix);
    }
}

} // namespace bassamp
