#include "bassamp/Tuner.h"

namespace bassamp {

static const TuningPreset kTunings[] =
{
    { "4-string Standard (EADG)", 4, { "E", "A", "D", "G", "", "" },      { 28, 33, 38, 43, 0, 0 } },
    { "5-string Standard (BEADG)",5, { "B", "E", "A", "D", "G", "" },     { 23, 28, 33, 38, 43, 0 } },
    { "5-string Tenor (EADGC)",   5, { "E", "A", "D", "G", "C", "" },     { 28, 33, 38, 43, 48, 0 } },
    { "6-string (BEADGC)",        6, { "B", "E", "A", "D", "G", "C" },    { 23, 28, 33, 38, 43, 48 } },
    { "Drop D",                   4, { "D", "A", "D", "G", "", "" },      { 26, 33, 38, 43, 0, 0 } },
    { "Drop C (5-string)",        5, { "A", "E", "A", "D", "G", "" },     { 21, 28, 33, 38, 43, 0 } },
    { "D Standard",               4, { "D", "G", "C", "F", "", "" },      { 26, 31, 36, 41, 0, 0 } },
    { "Half Step Down",           4, { "Eb", "Ab", "Db", "Gb", "", "" },  { 27, 32, 37, 42, 0, 0 } },
    { "BEAD (4-string)",          4, { "B", "E", "A", "D", "", "" },      { 23, 28, 33, 38, 0, 0 } },
    { "Piccolo (EADG +1 oct)",    4, { "E", "A", "D", "G", "", "" },      { 40, 45, 50, 55, 0, 0 } }
};

int numTuningPresets() { return (int) (sizeof (kTunings) / sizeof (TuningPreset)); }
const TuningPreset& tuningPreset (int index)
{
    return kTunings[std::max (0, std::min (index, numTuningPresets() - 1))];
}

std::string midiNoteName (int midiNote)
{
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    const int n = std::max (0, midiNote);
    const int octave = n / 12 - 1;
    return std::string (names[n % 12]) + std::to_string (octave);
}

void Tuner::prepare (double sampleRate, int maxBlockSize)
{
    baseRate = sampleRate;

    // Decimate to roughly 12 kHz. A bass fundamental tops out around 400 Hz, so
    // everything above 6 kHz is just cost, and YIN gets cheaper by the square of
    // the window length.
    decimation = std::max (1, (int) std::round (sampleRate / 12000.0));
    analysisRate = sampleRate / decimation;

    const float antiAlias = (float) (analysisRate * 0.42);
    decimateLp1.setLowpass (sampleRate, antiAlias, 0.54f);
    decimateLp2.setLowpass (sampleRate, antiAlias, 1.31f);

    // Band-limit before detection: the fundamental is what we are measuring, and
    // a bright bass has far more energy in its harmonics than at 41 Hz.
    inputBandpass.setLowpass (analysisRate, 900.0f, 0.707f);

    fifo.prepare (1 << 15);
    analysisWindow.assign (2048, 0.0f);
    yinDifference.assign (1024, 0.0f);

    strobeLpCoeff = timeConstantCoeff (0.050f, analysisRate);

    for (int b = 0; b < kStrobeBands; ++b) strobeAngle[b].store (0.0f);

    (void) maxBlockSize;
    reset();
}

void Tuner::reset()
{
    decimateLp1.reset(); decimateLp2.reset(); inputBandpass.reset();
    decimateCounter = 0;
    for (int b = 0; b < kStrobeBands; ++b) { strobePhase[b] = 0.0; strobeI[b] = strobeQ[b] = 0.0f; }
    strobeLevel = 0.0f;
    signalPresent.store (false);
    detectedClarity.store (0.0f);
}

float Tuner::midiToFrequency (float midiNote) const
{
    return referenceA.load() * std::pow (2.0f, (midiNote - 69.0f) / 12.0f);
}

void Tuner::processAudio (const float* data, int numSamples)
{
    for (int n = 0; n < numSamples; ++n)
    {
        const float filtered = decimateLp2.process (decimateLp1.process (data[n]));
        if (++decimateCounter >= decimation)
        {
            decimateCounter = 0;
            // YIN wants the fundamental isolated; the strobe wants the full
            // harmonic series, because its upper bands lock to the harmonics.
            const float forYin = inputBandpass.process (filtered);
            fifo.push (&forYin, 1);
            updateStrobe (filtered);
        }
    }
}

void Tuner::updateStrobe (float sample)
{
    const float target = midiToFrequency ((float) targetMidi.load());
    const float rect = std::fabs (sample);
    strobeLevel = (rect > strobeLevel) ? rect : strobeLevel * 0.9995f;

    for (int b = 0; b < kStrobeBands; ++b)
    {
        const double bandFreq = target * (double) (1 << b);
        if (bandFreq > analysisRate * 0.45) continue;   // band is above Nyquist here

        strobePhase[b] += 2.0 * kPiD * bandFreq / analysisRate;
        if (strobePhase[b] > 2.0 * kPiD) strobePhase[b] -= 2.0 * kPiD;

        const float c = (float) std::cos (strobePhase[b]);
        const float s = (float) std::sin (strobePhase[b]);

        // Quadrature demodulation. What survives the low-pass is a phasor whose
        // angle advances at (played pitch - reference pitch) Hz: in tune, it
        // stands still; sharp, it rotates one way; flat, the other.
        strobeI[b] = flushDenormal (strobeLpCoeff * strobeI[b] + (1.0f - strobeLpCoeff) * (sample * c));
        strobeQ[b] = flushDenormal (strobeLpCoeff * strobeQ[b] + (1.0f - strobeLpCoeff) * (-sample * s));

        strobeAngle[b].store (std::atan2 (strobeQ[b], strobeI[b]));
    }
}

float Tuner::getStrobePhase (int band) const
{
    if (band < 0 || band >= kStrobeBands) return 0.0f;
    const float angle = strobeAngle[band].load();
    return (angle + kPi) / kTwoPi;   // 0..1 for the UI to rotate
}

// YIN (de Cheveigne & Kawahara 2002). The cumulative mean normalisation is what
// makes it robust against the octave errors plain autocorrelation gives you on a
// harmonically rich instrument - which a bass very much is.
float Tuner::runYin (const float* window, int windowSize, int maxLag, float& clarityOut) const
{
    std::vector<float>* d = &yinDifference;
    if ((int) d->size() < maxLag + 1) d->assign ((size_t) maxLag + 1, 0.0f);

    (*d)[0] = 1.0f;

    // Difference function.
    for (int tau = 1; tau <= maxLag; ++tau)
    {
        float sum = 0.0f;
        for (int j = 0; j < windowSize; ++j)
        {
            const float delta = window[j] - window[j + tau];
            sum += delta * delta;
        }
        (*d)[(size_t) tau] = sum;
    }

    // Cumulative mean normalised difference.
    float runningSum = 0.0f;
    for (int tau = 1; tau <= maxLag; ++tau)
    {
        runningSum += (*d)[(size_t) tau];
        (*d)[(size_t) tau] = (runningSum > 1.0e-12f) ? (*d)[(size_t) tau] * (float) tau / runningSum : 1.0f;
    }

    // Absolute threshold: take the first dip below it rather than the global
    // minimum, which is what suppresses the octave-down error.
    const float threshold = 0.15f;
    int bestTau = -1;
    for (int tau = 2; tau < maxLag; ++tau)
    {
        if ((*d)[(size_t) tau] < threshold)
        {
            while (tau + 1 < maxLag && (*d)[(size_t) (tau + 1)] < (*d)[(size_t) tau]) ++tau;
            bestTau = tau;
            break;
        }
    }

    if (bestTau < 0)
    {
        float best = 1.0f;
        for (int tau = 2; tau < maxLag; ++tau)
            if ((*d)[(size_t) tau] < best) { best = (*d)[(size_t) tau]; bestTau = tau; }
        if (bestTau < 0 || best > 0.6f) { clarityOut = 0.0f; return 0.0f; }
    }

    clarityOut = 1.0f - (*d)[(size_t) bestTau];

    // Parabolic interpolation around the minimum. Without it the resolution is
    // limited to one sample of lag, which at 12 kHz is about 4 cents on a low B.
    float refined = (float) bestTau;
    if (bestTau > 1 && bestTau < maxLag - 1)
    {
        const float a = (*d)[(size_t) (bestTau - 1)];
        const float b = (*d)[(size_t) bestTau];
        const float c = (*d)[(size_t) (bestTau + 1)];
        const float denom = 2.0f * (2.0f * b - a - c);
        if (std::fabs (denom) > 1.0e-9f) refined = (float) bestTau + (c - a) / denom;
    }

    return (refined > 0.0f) ? (float) analysisRate / refined : 0.0f;
}

void Tuner::runDetection()
{
    // Window covers the lowest note we care about: a low B at 31 Hz needs more
    // than one period, and the lag search has to reach it too.
    const int maxLag = (int) (analysisRate / 28.0);
    const int windowSize = (int) (analysisRate / 12.0);   // ~83 ms of signal
    const int needed = windowSize + maxLag + 1;

    if ((int) analysisWindow.size() < needed) analysisWindow.assign ((size_t) needed, 0.0f);
    if (fifo.available() < needed) return;
    if (! fifo.readLatest (analysisWindow.data(), needed)) return;

    // Gate on level: without this the tuner chases noise between notes.
    float rms = 0.0f;
    for (int i = 0; i < needed; ++i) rms += analysisWindow[(size_t) i] * analysisWindow[(size_t) i];
    rms = std::sqrt (rms / (float) needed);

    if (rms < 0.0015f)
    {
        signalPresent.store (false);
        detectedClarity.store (0.0f);
        return;
    }

    float clarity = 0.0f;
    const float freq = runYin (analysisWindow.data(), windowSize, maxLag, clarity);

    if (freq < 25.0f || freq > 900.0f || clarity < 0.35f)
    {
        signalPresent.store (false);
        detectedClarity.store (clarity);
        return;
    }

    const float midiFloat = 69.0f + 12.0f * std::log2 (freq / referenceA.load());
    const int   nearest   = (int) std::lround (midiFloat);
    const float cents     = (midiFloat - (float) nearest) * 100.0f;

    detectedFrequency.store (freq);
    detectedMidi.store (nearest);
    detectedCents.store (cents);
    detectedClarity.store (clarity);
    signalPresent.store (true);

    if (autoTarget.load() && targetMidi.load() != nearest) targetMidi.store (nearest);
}

} // namespace bassamp
