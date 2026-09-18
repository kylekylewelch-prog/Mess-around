#include "bassamp/Pedals.h"

namespace bassamp {

// ============================================================================
//  Pedal base
// ============================================================================

void Pedal::setParamValue (int index, float v)
{
    if (index < 0 || index >= numParams()) return;
    const ParamDesc& d = param (index);
    const float clamped = clampf (v, d.minValue, d.maxValue);
    if (values[index] == clamped) return;
    values[index] = clamped;
    paramsChanged();
}

float Pedal::getParamValue (int index) const
{
    return (index >= 0 && index < numParams()) ? values[index] : 0.0f;
}

void Pedal::restoreDefaults()
{
    for (int i = 0; i < numParams(); ++i) values[i] = param (i).defaultValue;
    paramsChanged();
}

const char* const kDivisionNames[10] =
    { "1/1", "1/2", "1/4.", "1/4", "1/4T", "1/8.", "1/8", "1/8T", "1/16", "1/16T" };

const char* const* Pedal::divisionChoices()  { return kDivisionNames; }
int Pedal::divisionChoiceCount() { return 10; }

float Pedal::divisionToSeconds (int divisionIndex, double bpm)
{
    const double beat = 60.0 / std::max (20.0, bpm);   // one quarter note
    static const double mult[] = { 4.0, 2.0, 1.5, 1.0, 2.0 / 3.0, 0.75, 0.5, 1.0 / 3.0, 0.25, 1.0 / 6.0 };
    const int i = std::max (0, std::min (divisionIndex, 9));
    return (float) (beat * mult[i]);
}

// ============================================================================
//  Compressor
// ============================================================================

static const char* const kCompModes[] = { "Opto", "FET", "Bass" };

const ParamDesc CompressorPedal::descs[] =
{
    { "Threshold", -48.0f,   0.0f, -18.0f, "dB", 0, nullptr, 0.5f },
    { "Ratio",       1.0f,  20.0f,   4.0f, ":1", 0, nullptr, 0.35f },
    { "Attack",      0.5f, 100.0f,  12.0f, "ms", 0, nullptr, 0.3f },
    { "Release",    20.0f, 800.0f, 180.0f, "ms", 0, nullptr, 0.35f },
    { "Makeup",      0.0f,  24.0f,   6.0f, "dB", 0, nullptr, 0.5f },
    { "Mix",         0.0f, 100.0f, 100.0f, "%",  0, nullptr, 0.5f },
    { "Mode",        0.0f,   2.0f,   0.0f, "",   3, kCompModes, 0.5f }
};
BASSAMP_PEDAL_DEFS (CompressorPedal)

void CompressorPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    restoreDefaults();
    reset();
}

void CompressorPedal::reset()
{
    sidechainHp.reset();
    envDb = -100.0f;
    currentReductionDb = 0.0f;
}

void CompressorPedal::paramsChanged()
{
    attackCoeff  = timeConstantCoeff (paramAt (2) * 0.001f, fs);
    releaseCoeff = timeConstantCoeff (paramAt (3) * 0.001f, fs);

    // Bass mode listens above 150 Hz so a low B does not pump the whole signal
    // every time it comes round.
    const int mode = choiceAt (6);
    sidechainHp.setHighpass (fs, mode == 2 ? 150.0f : 20.0f, 0.707f);
}

void CompressorPedal::process (float* data, int numSamples)
{
    const float thresholdDb = paramAt (0);
    const float ratio       = std::max (1.0f, paramAt (1));
    const float makeup      = dbToGain (paramAt (4));
    const float mix         = paramAt (5) * 0.01f;
    const int   mode        = choiceAt (6);
    const float knee        = (mode == 1) ? 2.0f : 8.0f;   // FET is a harder knee

    for (int n = 0; n < numSamples; ++n)
    {
        const float dry = data[n];
        const float detect = sidechainHp.process (dry);
        const float levelDb = gainToDb (std::fabs (detect));

        // Static compression curve with a soft knee.
        float targetReduction = 0.0f;
        const float over = levelDb - thresholdDb;
        if (over > knee * 0.5f)
            targetReduction = over - over / ratio;
        else if (over > -knee * 0.5f)
        {
            const float t = over + knee * 0.5f;
            targetReduction = (1.0f - 1.0f / ratio) * t * t / (2.0f * knee);
        }

        // Opto cells release more slowly the harder they have been driven, which
        // is why an opto compressor sounds "musical" on a bass line.
        float release = releaseCoeff;
        if (mode == 0)
        {
            const float programFactor = 1.0f + 2.0f * clampf (currentReductionDb / 12.0f, 0.0f, 1.0f);
            release = timeConstantCoeff (paramAt (3) * 0.001f * programFactor, fs);
        }

        const float coeff = (targetReduction > currentReductionDb) ? attackCoeff : release;
        currentReductionDb = flushDenormal (coeff * currentReductionDb + (1.0f - coeff) * targetReduction);

        const float wet = dry * dbToGain (-currentReductionDb) * makeup;
        data[n] = wet * mix + dry * (1.0f - mix);
    }
    envDb = currentReductionDb;
}

// ============================================================================
//  Noise gate
// ============================================================================

const ParamDesc NoiseGatePedal::descs[] =
{
    { "Threshold", -80.0f,  -10.0f, -52.0f, "dB", 0, nullptr, 0.5f },
    { "Attack",      0.2f,   50.0f,   2.0f, "ms", 0, nullptr, 0.3f },
    { "Hold",        0.0f,  500.0f,  60.0f, "ms", 0, nullptr, 0.4f },
    { "Release",    10.0f, 1000.0f, 220.0f, "ms", 0, nullptr, 0.35f },
    { "Range",       6.0f,   80.0f,  60.0f, "dB", 0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (NoiseGatePedal)

void NoiseGatePedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    detector.prepare (sampleRate);
    restoreDefaults();
    reset();
}

void NoiseGatePedal::reset()
{
    detector.reset();
    currentGain = 1.0f;
    holdCounter = 0;
    open = false;
}

void NoiseGatePedal::paramsChanged()
{
    detector.setTimes (0.001f, 0.020f);
    attackCoeff  = timeConstantCoeff (paramAt (1) * 0.001f, fs);
    releaseCoeff = timeConstantCoeff (paramAt (3) * 0.001f, fs);
    holdSamples  = (int) (paramAt (2) * 0.001f * fs);
}

void NoiseGatePedal::process (float* data, int numSamples)
{
    const float openThresh  = dbToGain (paramAt (0));
    // 6 dB of hysteresis: without it the gate chatters on a decaying note.
    const float closeThresh = dbToGain (paramAt (0) - 6.0f);
    const float floorGain   = dbToGain (-paramAt (4));

    for (int n = 0; n < numSamples; ++n)
    {
        const float env = detector.process (data[n]);

        if (! open && env > openThresh) { open = true; holdCounter = holdSamples; }
        else if (open && env < closeThresh)
        {
            if (holdCounter > 0) --holdCounter;
            else open = false;
        }
        else if (open && env >= closeThresh) holdCounter = holdSamples;

        const float target = open ? 1.0f : floorGain;
        const float coeff = (target > currentGain) ? attackCoeff : releaseCoeff;
        currentGain = flushDenormal (coeff * currentGain + (1.0f - coeff) * target);

        data[n] *= currentGain;
    }
}

// ============================================================================
//  Graphic EQ
// ============================================================================

static const float kEqFrequencies[7] = { 50.0f, 120.0f, 400.0f, 800.0f, 1600.0f, 4000.0f, 10000.0f };

const ParamDesc GraphicEqPedal::descs[] =
{
    { "50 Hz",  -12.0f, 12.0f, 0.0f, "dB", 0, nullptr, 0.5f },
    { "120 Hz", -12.0f, 12.0f, 0.0f, "dB", 0, nullptr, 0.5f },
    { "400 Hz", -12.0f, 12.0f, 0.0f, "dB", 0, nullptr, 0.5f },
    { "800 Hz", -12.0f, 12.0f, 0.0f, "dB", 0, nullptr, 0.5f },
    { "1.6 kHz",-12.0f, 12.0f, 0.0f, "dB", 0, nullptr, 0.5f },
    { "4 kHz",  -12.0f, 12.0f, 0.0f, "dB", 0, nullptr, 0.5f },
    { "10 kHz", -12.0f, 12.0f, 0.0f, "dB", 0, nullptr, 0.5f }
};
BASSAMP_PEDAL_DEFS (GraphicEqPedal)

void GraphicEqPedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    restoreDefaults();
    reset();
}

void GraphicEqPedal::reset() { for (auto& b : bands) b.reset(); }

void GraphicEqPedal::paramsChanged()
{
    // Q of 1.4 gives roughly one-octave bands that sum sensibly when adjacent
    // sliders move together.
    for (int i = 0; i < kBands; ++i)
        bands[i].setPeaking (fs, kEqFrequencies[i], 1.4f, paramAt (i));
}

void GraphicEqPedal::process (float* data, int numSamples)
{
    for (int n = 0; n < numSamples; ++n)
    {
        float x = data[n];
        for (auto& b : bands) x = b.process (x);
        data[n] = x;
    }
}

float GraphicEqPedal::magnitudeDbAt (float freq) const
{
    float m = 1.0f;
    for (auto& b : bands) m *= b.magnitudeAt (freq, fs);
    return gainToDb (m);
}

// ============================================================================
//  Octave
// ============================================================================

const ParamDesc OctavePedal::descs[] =
{
    { "Direct",  0.0f, 100.0f, 100.0f, "%",  0, nullptr, 0.5f },
    { "Oct Down",0.0f, 100.0f,  70.0f, "%",  0, nullptr, 0.5f },
    { "2 Oct",   0.0f, 100.0f,   0.0f, "%",  0, nullptr, 0.5f },
    { "Oct Up",  0.0f, 100.0f,   0.0f, "%",  0, nullptr, 0.5f },
    { "Tone",  300.0f, 6000.0f, 1600.0f, "Hz", 0, nullptr, 0.3f }
};
BASSAMP_PEDAL_DEFS (OctavePedal)

void OctavePedal::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate; maxBlock = maxBlockSize;
    divider.prepare (sampleRate);
    restoreDefaults();
    reset();
}

void OctavePedal::reset() { divider.reset(); toneLp.reset(); }

void OctavePedal::paramsChanged()
{
    toneLp.setLowpass (fs, paramAt (4), 0.707f);
}

void OctavePedal::process (float* data, int numSamples)
{
    const float direct = paramAt (0) * 0.01f;
    const float d1 = paramAt (1) * 0.01f;
    const float d2 = paramAt (2) * 0.01f;
    const float up = paramAt (3) * 0.01f;

    for (int n = 0; n < numSamples; ++n)
    {
        const float x = data[n];
        const auto o = divider.process (x);
        const float generated = toneLp.process (o.down1 * d1 * 1.6f + o.down2 * d2 * 1.6f + o.up * up * 0.8f);
        data[n] = x * direct + generated;
    }
}

} // namespace bassamp
