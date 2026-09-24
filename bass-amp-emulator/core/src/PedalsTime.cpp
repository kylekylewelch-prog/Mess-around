// Delay and reverb.
#include "bassamp/Pedals.h"

namespace bassamp {

// ============================================================================
//  Delay
// ============================================================================

static const char* const kDelayModes[] = { "Digital", "Analog", "Tape" };
static const char* const kDelaySync[]  = { "Free", "Tempo" };

const ParamDesc DelayPedal::descs[] =
{
    { "Time",     20.0f, 2000.0f, 380.0f, "ms", 0, nullptr, 0.35f },
    { "Feedback",  0.0f,   95.0f,  35.0f, "%",  0, nullptr, 0.5f },
    { "Mix",       0.0f,  100.0f,  25.0f, "%",  0, nullptr, 0.5f },
    { "Tone",    800.0f, 12000.0f, 4000.0f, "Hz", 0, nullptr, 0.3f },
    { "Mode",      0.0f,    2.0f,   0.0f, "",   3, kDelayModes, 0.5f },
    { "Sync",      0.0f,    1.0f,   0.0f, "",   2, kDelaySync, 0.5f },
    { "Division",  0.0f,    9.0f,   6.0f, "",  10, kDivisionNames, 0.5f }
};
BASSAMP_PEDAL_DEFS (DelayPedal)

void DelayPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    line.prepare (sampleRate, 2.2f);
    wow.prepare (sampleRate);
    flutter.prepare (sampleRate);
    wow.setRate (0.6f);
    flutter.setRate (7.3f);
    // Smoothing the delay time turns a knob twist into a tape-style pitch glide
    // instead of a click.
    timeSmooth.reset (sampleRate, 0.15f, 0.38f * (float) sampleRate);
    restoreDefaults();
    reset();
}

void DelayPedal::reset()
{
    line.reset(); repeatLp.reset(); repeatHp.reset();
    wow.reset(); flutter.reset();
    feedbackState = 0.0f;
}

void DelayPedal::paramsChanged()
{
    const int mode = choiceAt (4);
    // Analog (BBD) delays lose top end on every pass; tape loses top and bottom.
    const float toneHz = (mode == 0) ? paramAt (3)
                       : (mode == 1) ? std::min (paramAt (3), 3200.0f)
                                     : std::min (paramAt (3), 5000.0f);
    repeatLp.setLowpass (fs, toneHz, 0.707f);
    repeatHp.setHighpass (fs, mode == 2 ? 120.0f : (mode == 1 ? 80.0f : 25.0f), 0.707f);
}

void DelayPedal::process (float* data, int numSamples)
{
    const int mode = choiceAt (4);
    const float feedback = paramAt (1) * 0.01f;
    const float mix = paramAt (2) * 0.01f;

    const float timeSeconds = (choiceAt (5) == 1) ? divisionToSeconds (choiceAt (6), tempoBpm)
                                                  : paramAt (0) * 0.001f;
    timeSmooth.setTarget (clampf (timeSeconds * (float) fs, 4.0f, (float) line.getMaxDelaySamples()));

    for (int n = 0; n < numSamples; ++n)
    {
        const float dry = data[n];

        float delaySamples = timeSmooth.next();
        if (mode == 2)
        {
            // Wow is the slow drift of a worn capstan, flutter the faster
            // scrape. Both are tiny: a few hundred microseconds.
            const float w = wow.next (Lfo::Shape::Sine) * 0.0025f;
            const float f = flutter.next (Lfo::Shape::Triangle) * 0.0006f;
            delaySamples *= (1.0f + w + f);
        }

        const float wet = line.read (delaySamples);

        float fb = repeatHp.process (repeatLp.process (wet)) * feedback;
        if (mode != 0) fb = fastTanh (fb * 1.2f) * 0.85f;   // the repeats degrade
        feedbackState = flushDenormal (fb);

        line.write (dry + feedbackState);
        data[n] = dry + wet * mix;
    }
}

// ============================================================================
//  Reverb
// ============================================================================

const ParamDesc ReverbPedal::descs[] =
{
    { "Size",     0.0f, 100.0f, 45.0f, "%",  0, nullptr, 0.5f },
    { "Damping",  0.0f, 100.0f, 55.0f, "%",  0, nullptr, 0.5f },
    { "Pre-Delay",0.0f, 120.0f, 15.0f, "ms", 0, nullptr, 0.5f },
    { "Low Cut", 40.0f, 600.0f, 220.0f, "Hz", 0, nullptr, 0.4f },
    { "Mix",      0.0f, 100.0f, 18.0f, "%",  0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (ReverbPedal)

void ReverbPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;

    // Mutually prime delay lengths so the modal density builds instead of the
    // taps piling up on each other.
    static const float baseMs[kLines] = { 43.7f, 59.3f, 71.9f, 89.1f };
    for (int i = 0; i < kLines; ++i)
    {
        lines[i].prepare (sampleRate, 0.30f);
        lineLengths[i] = baseMs[i] * 0.001f * (float) sampleRate;
    }
    diffusion[0].prepare (sampleRate, 0.02f);
    diffusion[1].prepare (sampleRate, 0.02f);
    preDelay.prepare (sampleRate, 0.20f);

    restoreDefaults();
    reset();
}

void ReverbPedal::reset()
{
    for (int i = 0; i < kLines; ++i) { lines[i].reset(); damping[i].reset(); feedbackState[i] = 0.0f; }
    diffusion[0].reset(); diffusion[1].reset(); preDelay.reset(); inputHp.reset();
}

void ReverbPedal::paramsChanged()
{
    const float dampHz = logRange (1.0f - paramAt (1) * 0.01f, 700.0f, 11000.0f);
    for (int i = 0; i < kLines; ++i) damping[i].setLowpass (fs, dampHz, 0.707f);
    // Keeping the fundamental out of the tank is the difference between ambience
    // and mud on a bass guitar.
    inputHp.setHighpass (fs, paramAt (3), 0.707f);
}

void ReverbPedal::process (float* data, int numSamples)
{
    const float size = 0.55f + paramAt (0) * 0.0075f;          // scales the tank
    const float decay = 0.62f + paramAt (0) * 0.0032f;         // and its feedback
    const float preDelaySamples = std::max (1.0f, paramAt (2) * 0.001f * (float) fs);
    const float mix = paramAt (4) * 0.01f;

    for (int n = 0; n < numSamples; ++n)
    {
        const float dry = data[n];

        preDelay.write (inputHp.process (dry));
        float x = preDelay.read (preDelaySamples);

        // Two all-pass diffusers smear the transient before it enters the tank.
        for (int d = 0; d < 2; ++d)
        {
            const float delaySamples = (d == 0 ? 0.0071f : 0.0113f) * (float) fs;
            const float delayed = diffusion[d].read (delaySamples);
            const float v = x - 0.62f * delayed;
            diffusion[d].write (v);
            x = delayed + 0.62f * v;
        }

        // Householder feedback matrix: lossless mixing, so the tail decays only
        // by the amount the damping filters and the decay gain take out.
        const float a = feedbackState[0], b = feedbackState[1], c = feedbackState[2], d4 = feedbackState[3];
        const float sum = 0.5f * (a + b + c + d4);
        const float mixed[kLines] = { a - sum, b - sum, c - sum, d4 - sum };

        float wet = 0.0f;
        for (int i = 0; i < kLines; ++i)
        {
            lines[i].write (x + mixed[i] * decay);
            const float out = lines[i].read (lineLengths[i] * size);
            feedbackState[i] = flushDenormal (damping[i].process (out));
            wet += out * (i % 2 == 0 ? 0.5f : -0.5f);
        }

        data[n] = dry * (1.0f - mix) + wet * mix;
    }
}

} // namespace bassamp
