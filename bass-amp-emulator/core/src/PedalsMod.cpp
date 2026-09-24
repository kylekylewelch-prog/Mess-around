// Filter and modulation pedals.
#include "bassamp/Pedals.h"

namespace bassamp {

// ============================================================================
//  Wah
// ============================================================================

static const char* const kWahModes[] = { "Pedal", "Auto LFO" };

const ParamDesc WahPedal::descs[] =
{
    { "Pedal",     0.0f, 100.0f,  50.0f, "%",  0, nullptr, 0.5f },
    { "Heel",    180.0f, 800.0f, 320.0f, "Hz", 0, nullptr, 0.35f },
    { "Toe",     800.0f, 3500.0f, 1900.0f, "Hz", 0, nullptr, 0.35f },
    { "Q",         1.0f,  12.0f,   4.5f, "",   0, nullptr, 0.4f },
    { "Mode",      0.0f,   1.0f,   0.0f, "",   2, kWahModes, 0.5f },
    { "Rate",      0.1f,   8.0f,   1.2f, "Hz", 0, nullptr, 0.35f },
    { "Mix",       0.0f, 100.0f, 100.0f, "%",  0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (WahPedal)

void WahPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    svf.prepare (sampleRate);
    lfo.prepare (sampleRate);
    pedalSmooth.reset (sampleRate, 0.015f, 0.5f);
    restoreDefaults();
    reset();
}

void WahPedal::reset() { svf.reset(); lfo.reset(); outputTilt.reset(); }

void WahPedal::paramsChanged()
{
    // The original circuit loses low end as it sweeps up; without this it sounds
    // like an EQ sweep rather than a wah.
    outputTilt.setHighShelf (fs, 1200.0f, 0.7f, 3.0f);
    lfo.setRate (paramAt (5));
}

void WahPedal::process (float* data, int numSamples)
{
    const float heel = paramAt (1);
    const float toe  = std::max (paramAt (2), heel * 1.2f);
    const float q    = paramAt (3);
    const int   mode = choiceAt (4);
    const float mix  = paramAt (6) * 0.01f;

    pedalSmooth.setTarget (paramAt (0) * 0.01f);

    for (int n = 0; n < numSamples; ++n)
    {
        float position = pedalSmooth.next();
        if (mode == 1) position = 0.5f + 0.5f * lfo.next (Lfo::Shape::Sine);

        // Sweep is logarithmic: a linear sweep sounds bottom-heavy because pitch
        // perception is logarithmic too.
        const float cutoff = logRange (clampf (position, 0.0f, 1.0f), heel, toe);
        svf.setParams (cutoff, q);

        const float dry = data[n];
        const auto out = svf.process (dry);
        // Band-pass with a touch of the high-pass gives the vocal formant quality
        // the pure band-pass lacks.
        const float wet = outputTilt.process (out.bp * 1.6f + out.hp * 0.25f);
        data[n] = wet * mix + dry * (1.0f - mix);
    }
}

// ============================================================================
//  Envelope filter
// ============================================================================

static const char* const kEnvModes[]      = { "Low Pass", "Band Pass", "High Pass" };
static const char* const kEnvDirections[] = { "Up", "Down" };

const ParamDesc EnvelopeFilterPedal::descs[] =
{
    { "Sensitivity", 0.0f, 100.0f,  60.0f, "%",  0, nullptr, 0.5f },
    { "Range",     100.0f, 1200.0f, 260.0f, "Hz", 0, nullptr, 0.35f },
    { "Peak",        1.0f,  14.0f,   5.0f, "Q",  0, nullptr, 0.4f },
    { "Attack",      1.0f,  60.0f,   8.0f, "ms", 0, nullptr, 0.35f },
    { "Decay",      30.0f, 600.0f, 160.0f, "ms", 0, nullptr, 0.35f },
    { "Mode",        0.0f,   2.0f,   1.0f, "",   3, kEnvModes, 0.5f },
    { "Direction",   0.0f,   1.0f,   0.0f, "",   2, kEnvDirections, 0.5f },
    { "Mix",         0.0f, 100.0f, 100.0f, "%",  0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (EnvelopeFilterPedal)

void EnvelopeFilterPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    svf.prepare (sampleRate);
    follower.prepare (sampleRate);
    restoreDefaults();
    reset();
}

void EnvelopeFilterPedal::reset() { svf.reset(); follower.reset(); sweepDisplay = 0.0f; }

void EnvelopeFilterPedal::paramsChanged()
{
    follower.setTimes (paramAt (3) * 0.001f, paramAt (4) * 0.001f);
}

void EnvelopeFilterPedal::process (float* data, int numSamples)
{
    const float sensitivity = 0.5f + paramAt (0) * 0.18f;
    const float range = paramAt (1);
    const float q = paramAt (2);
    const int mode = choiceAt (5);
    const bool downward = choiceAt (6) == 1;
    const float mix = paramAt (7) * 0.01f;

    for (int n = 0; n < numSamples; ++n)
    {
        const float dry = data[n];
        const float env = clampf (follower.process (dry) * sensitivity, 0.0f, 1.0f);

        // Four octaves of sweep, up or down from the Range setting.
        const float octaves = downward ? -3.2f * env : 3.6f * env;
        const float cutoff = clampf (range * std::pow (2.0f, octaves), 60.0f, (float) fs * 0.45f);
        svf.setParams (cutoff, q);

        const auto out = svf.process (dry);
        const float wet = (mode == 0) ? out.lp : (mode == 1) ? out.bp * 1.5f : out.hp;
        data[n] = wet * mix + dry * (1.0f - mix);
        sweepDisplay = env;
    }
}

// ============================================================================
//  Chorus
// ============================================================================

static const char* const kChorusVoices[] = { "1", "2", "3" };

const ParamDesc ChorusPedal::descs[] =
{
    { "Rate",      0.05f,  6.0f,   0.6f, "Hz", 0, nullptr, 0.35f },
    { "Depth",     0.0f, 100.0f,  40.0f, "%",  0, nullptr, 0.5f },
    { "Delay",     5.0f,  35.0f,  16.0f, "ms", 0, nullptr, 0.5f },
    { "Voices",    0.0f,   2.0f,   1.0f, "",   3, kChorusVoices, 0.5f },
    { "Low Keep", 40.0f, 400.0f, 120.0f, "Hz", 0, nullptr, 0.4f },
    { "Mix",       0.0f, 100.0f,  35.0f, "%",  0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (ChorusPedal)

void ChorusPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    line.prepare (sampleRate, 0.09f);
    lfo.prepare (sampleRate);
    split.prepare (sampleRate);
    restoreDefaults();
    reset();
}

void ChorusPedal::reset() { line.reset(); lfo.reset(); split.reset(); }

void ChorusPedal::paramsChanged()
{
    lfo.setRate (paramAt (0));
    split.setFrequency (paramAt (4));
}

void ChorusPedal::process (float* data, int numSamples)
{
    const float depthMs = paramAt (1) * 0.01f * 7.0f;      // up to 7 ms of swing
    const float centreMs = paramAt (2);
    const int voices = std::max (1, std::min (3, choiceAt (3) + 1));
    const float mix = paramAt (5) * 0.01f;

    const float centre = centreMs * 0.001f * (float) fs;
    const float depth  = depthMs  * 0.001f * (float) fs;

    for (int n = 0; n < numSamples; ++n)
    {
        const float dry = data[n];
        float low, high;
        split.crossover.process (dry, low, high);

        line.write (high);
        lfo.next (Lfo::Shape::Sine);          // advance the shared phase
        const float phase = lfo.getPhase();

        float wet = 0.0f;
        for (int v = 0; v < voices; ++v)
        {
            // Spread the voices evenly round the cycle so they never line up.
            const float mod = std::sin (kTwoPi * (phase + (float) v / (float) voices));
            wet += line.read (centre + depth * mod);
        }
        wet /= (float) voices;

        data[n] = low + high * (1.0f - mix) + wet * mix;
    }
}

// ============================================================================
//  Flanger
// ============================================================================

const ParamDesc FlangerPedal::descs[] =
{
    { "Rate",     0.02f,  5.0f,  0.25f, "Hz", 0, nullptr, 0.3f },
    { "Depth",     0.0f, 100.0f, 60.0f, "%",  0, nullptr, 0.5f },
    { "Manual",    0.2f,   9.0f,  1.2f, "ms", 0, nullptr, 0.4f },
    { "Feedback", -95.0f, 95.0f, 55.0f, "%",  0, nullptr, 0.5f },
    { "Mix",       0.0f, 100.0f, 50.0f, "%",  0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (FlangerPedal)

void FlangerPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    line.prepare (sampleRate, 0.03f);
    lfo.prepare (sampleRate);
    restoreDefaults();
    reset();
}

void FlangerPedal::reset() { line.reset(); lfo.reset(); feedbackLp.reset(); feedbackState = 0.0f; }

void FlangerPedal::paramsChanged()
{
    lfo.setRate (paramAt (0));
    // Rolling the top off the feedback path stops it screaming at high regen.
    feedbackLp.setLowpass (fs, 6000.0f, 0.707f);
}

void FlangerPedal::process (float* data, int numSamples)
{
    const float depth = paramAt (1) * 0.01f;
    const float manualMs = paramAt (2);
    const float feedback = clampf (paramAt (3) * 0.01f, -0.95f, 0.95f);
    const float mix = paramAt (4) * 0.01f;

    const float base = manualMs * 0.001f * (float) fs;
    const float swing = base * 3.5f * depth;

    for (int n = 0; n < numSamples; ++n)
    {
        const float dry = data[n];
        const float mod = lfo.next (Lfo::Shape::Triangle);
        const float delaySamples = std::max (1.5f, base + swing * 0.5f * (mod + 1.0f));

        line.write (dry + feedbackState * feedback);
        const float wet = line.read (delaySamples);
        feedbackState = flushDenormal (feedbackLp.process (wet));

        data[n] = dry * (1.0f - mix * 0.5f) + wet * mix;
    }
}

// ============================================================================
//  Phaser
// ============================================================================

static const char* const kPhaserStages[] = { "4", "6", "8", "12" };

const ParamDesc PhaserPedal::descs[] =
{
    { "Rate",     0.02f,  6.0f,  0.45f, "Hz", 0, nullptr, 0.3f },
    { "Depth",     0.0f, 100.0f, 70.0f, "%",  0, nullptr, 0.5f },
    { "Centre",  120.0f, 1600.0f, 420.0f, "Hz", 0, nullptr, 0.35f },
    { "Feedback", 0.0f,  90.0f,  45.0f, "%",  0, nullptr, 0.5f },
    { "Stages",   0.0f,   3.0f,   1.0f, "",   4, kPhaserStages, 0.5f },
    { "Mix",      0.0f, 100.0f,  50.0f, "%",  0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (PhaserPedal)

void PhaserPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    lfo.prepare (sampleRate);
    restoreDefaults();
    reset();
}

void PhaserPedal::reset()
{
    for (auto& s : stages) s.reset();
    lfo.reset();
    feedbackState = 0.0f;
}

void PhaserPedal::paramsChanged() { lfo.setRate (paramAt (0)); }

void PhaserPedal::process (float* data, int numSamples)
{
    static const int stageCounts[] = { 4, 6, 8, 12 };
    const int numStages = stageCounts[std::max (0, std::min (choiceAt (4), 3))];
    const float depth = paramAt (1) * 0.01f;
    const float centre = paramAt (2);
    const float feedback = paramAt (3) * 0.01f * 0.85f;
    const float mix = paramAt (5) * 0.01f;

    for (int n = 0; n < numSamples; ++n)
    {
        const float dry = data[n];
        const float mod = lfo.next (Lfo::Shape::Sine);
        const float freq = clampf (centre * std::pow (2.0f, 2.2f * depth * mod), 40.0f, (float) fs * 0.45f);

        // Retuning every all-pass every sample is the honest way to do this; at
        // two coefficient updates per stage it is still cheap.
        for (int s = 0; s < numStages; ++s) stages[s].setAllpass (fs, freq, 0.707f);

        float y = dry + feedbackState * feedback;
        for (int s = 0; s < numStages; ++s) y = stages[s].process (y);
        feedbackState = flushDenormal (y);

        data[n] = dry * (1.0f - mix) + y * mix;
    }
}

// ============================================================================
//  Tremolo
// ============================================================================

static const char* const kTremShapes[] = { "Sine", "Triangle", "Square" };
static const char* const kSyncChoices[] = { "Free", "Tempo" };

const ParamDesc TremoloPedal::descs[] =
{
    { "Rate",  0.1f, 16.0f,  4.5f, "Hz", 0, nullptr, 0.35f },
    { "Depth", 0.0f, 100.0f, 55.0f, "%", 0, nullptr, 0.5f },
    { "Shape", 0.0f,   2.0f,  0.0f, "",  3, kTremShapes, 0.5f },
    { "Sync",  0.0f,   1.0f,  0.0f, "",  2, kSyncChoices, 0.5f },
    { "Division", 0.0f, 9.0f, 6.0f, "",  10, kDivisionNames, 0.5f }
};
BASSAMP_PEDAL_DEFS (TremoloPedal)

void TremoloPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    lfo.prepare (sampleRate);
    restoreDefaults();
    reset();
}

void TremoloPedal::reset() { lfo.reset(); }

void TremoloPedal::paramsChanged() { }

void TremoloPedal::process (float* data, int numSamples)
{
    const float depth = paramAt (1) * 0.01f;
    const int shapeIndex = choiceAt (2);
    const Lfo::Shape shape = shapeIndex == 0 ? Lfo::Shape::Sine
                           : shapeIndex == 1 ? Lfo::Shape::Triangle : Lfo::Shape::Square;

    if (choiceAt (3) == 1) lfo.setRate (1.0f / std::max (0.01f, divisionToSeconds (choiceAt (4), tempoBpm)));
    else                   lfo.setRate (paramAt (0));

    for (int n = 0; n < numSamples; ++n)
    {
        const float mod = 0.5f * (lfo.next (shape) + 1.0f);
        data[n] *= 1.0f - depth * (1.0f - mod);
    }
}

} // namespace bassamp
