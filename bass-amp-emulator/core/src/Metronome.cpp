#include "bassamp/Metronome.h"

namespace bassamp {

const char* subdivisionName (Subdivision s)
{
    switch (s)
    {
        case Subdivision::Quarter:   return "Quarter";
        case Subdivision::Eighth:    return "Eighth";
        case Subdivision::Triplet:   return "Triplet";
        case Subdivision::Sixteenth: return "Sixteenth";
        default:                     return "?";
    }
}

void Metronome::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate;
    for (int i = 0; i < kMaxBeats; ++i) accents[i].store ((int) (i == 0 ? AccentLevel::Accent : AccentLevel::Normal));
    clickBody.setBandpass (sampleRate, 2400.0f, 1.2f);
    tapTimes.reserve (8);
    (void) maxBlockSize;
    recalculate();
    reset();
}

// Schedule state belongs to the audio thread; the UI asks for a restart and the
// audio thread performs it at the next block boundary.
void Metronome::reset()
{
    restartRequested.store (true);
    currentBeat.store (0);
    beatProgress.store (0.0f);
}

int Metronome::getBeatsPerBar() const
{
    const TimeSignature ts = getTimeSignature();
    // A compound bar is counted in dotted-quarter pulses: 6/8 is two, not six.
    return ts.isCompound() ? ts.numerator / 3 : ts.numerator;
}

TimeSignature Metronome::getTimeSignature() const
{
    TimeSignature ts;
    ts.numerator = numerator.load();
    ts.denominator = denominator.load();
    return ts;
}

void Metronome::setTimeSignature (int num, int den)
{
    numerator.store (std::max (1, std::min (num, kMaxBeats)));
    const int d = (den == 1 || den == 2 || den == 4 || den == 8 || den == 16) ? den : 4;
    denominator.store (d);
}

void Metronome::setTempo (double bpm)
{
    tempoBpm.store (clampf ((float) bpm, 20.0f, 300.0f));
}

void Metronome::setRunning (bool shouldRun)
{
    if (shouldRun == running.load()) return;
    if (shouldRun)
    {
        countInRemaining.store (countInBars.load() * getBeatsPerBar());
        reset();
    }
    running.store (shouldRun);
}

void Metronome::setAccent (int beatIndex, AccentLevel a)
{
    if (beatIndex < 0 || beatIndex >= kMaxBeats) return;
    accents[beatIndex].store ((int) a);
}

AccentLevel Metronome::getAccent (int beatIndex) const
{
    if (beatIndex < 0 || beatIndex >= kMaxBeats) return AccentLevel::Normal;
    return (AccentLevel) accents[beatIndex].load();
}

void Metronome::recalculate()
{
    const TimeSignature ts = getTimeSignature();
    const double bpm = tempoBpm.load();

    // One "beat" is what the tempo refers to: a quarter note normally, a dotted
    // quarter in compound time.
    double beatSeconds = 60.0 / bpm;
    if (ts.isCompound()) beatSeconds *= 1.5;

    switch ((Subdivision) subdivision.load())
    {
        case Subdivision::Quarter:   ticksPerBeat = ts.isCompound() ? 3 : 1; break;
        case Subdivision::Eighth:    ticksPerBeat = ts.isCompound() ? 3 : 2; break;
        case Subdivision::Triplet:   ticksPerBeat = 3; break;
        case Subdivision::Sixteenth: ticksPerBeat = ts.isCompound() ? 6 : 4; break;
        default:                     ticksPerBeat = 1; break;
    }

    samplesPerTick = beatSeconds * fs / (double) ticksPerBeat;
}

void Metronome::tap (double timeSeconds)
{
    // Drop the history if the player stopped tapping: anything over two seconds
    // is a new tempo, not the next tap.
    if (! tapTimes.empty() && timeSeconds - tapTimes.back() > 2.0) tapTimes.clear();

    tapTimes.push_back (timeSeconds);
    if (tapTimes.size() > 5) tapTimes.erase (tapTimes.begin());
    if (tapTimes.size() < 2) return;

    double total = 0.0;
    for (size_t i = 1; i < tapTimes.size(); ++i) total += tapTimes[i] - tapTimes[i - 1];
    const double average = total / (double) (tapTimes.size() - 1);
    if (average > 0.15 && average < 3.0) setTempo (60.0 / average);
}

void Metronome::triggerClick (AccentLevel accentLevel, bool isSub)
{
    if (accentLevel == AccentLevel::Silent) return;

    // Three voices, pitched so you can hear the difference without looking:
    // downbeat highest, subdivisions lowest and quietest.
    float freq = 1000.0f, amplitude = 0.7f, decaySeconds = 0.045f;
    if (accentLevel == AccentLevel::Accent) { freq = 1600.0f; amplitude = 1.0f; decaySeconds = 0.055f; }
    if (isSub)                              { freq = 760.0f;  amplitude = 0.38f; decaySeconds = 0.030f; }

    clickIncrement = 2.0 * kPiD * freq / fs;
    // Start at the peak of the cycle rather than at a zero crossing: the click
    // is percussive, so its first sample should be its loudest. Starting from
    // zero puts the audible onset a sample or two late, which is exactly the
    // kind of small, consistent error that makes a metronome feel behind.
    clickPhase = kPiD * 0.5;
    clickEnv = amplitude;
    clickDecay = timeConstantCoeff (decaySeconds, fs);
    clickNoise = amplitude * 0.9f;   // the transient that makes it cut through
}

void Metronome::process (float* data, int numSamples)
{
    recalculate();

    if (restartRequested.exchange (false))
    {
        tickPhase = 0.0;
        tickInBeat = 0;
        beatInBar = 0;
        firstTick = true;
        clickEnv = 0.0f;
        clickPhase = 0.0;
        clickBody.reset();
    }

    const bool isRunningNow = running.load();
    const float gain = level.load();
    const float swingAmount = swing.load();
    const int beatsPerBar = getBeatsPerBar();

    for (int n = 0; n < numSamples; ++n)
    {
        if (isRunningNow)
        {
            // Swing pushes the off-ticks late by a fraction of a tick.
            double tickTarget = samplesPerTick;
            if (swingAmount > 0.001f && ticksPerBeat > 1)
                tickTarget *= (tickInBeat % 2 == 0) ? (1.0 + swingAmount) : (1.0 - swingAmount);

            if (firstTick)
            {
                // Start on the downbeat, on the sample the transport starts -
                // not one beat later.
                firstTick = false;
                tickPhase = 0.0;
                tickInBeat = 0;
                beatInBar = 0;
                currentBeat.store (0);
                const int remaining = countInRemaining.load();
                if (remaining > 0) countInRemaining.store (remaining - 1);
                triggerClick (getAccent (0), false);
            }
            else if (tickPhase >= tickTarget)
            {
                tickPhase -= tickTarget;
                ++tickInBeat;

                if (tickInBeat >= ticksPerBeat)
                {
                    tickInBeat = 0;
                    beatInBar = (beatInBar + 1) % std::max (1, beatsPerBar);
                    currentBeat.store (beatInBar);

                    const int remaining = countInRemaining.load();
                    if (remaining > 0) countInRemaining.store (remaining - 1);

                    triggerClick (getAccent (beatInBar), false);
                }
                else
                {
                    triggerClick (AccentLevel::Normal, true);
                }
            }

            tickPhase += 1.0;
            beatProgress.store ((float) ((tickInBeat + tickPhase / std::max (1.0, samplesPerTick)) / (double) ticksPerBeat));
        }

        if (clickEnv > 1.0e-5f)
        {
            noiseState ^= noiseState << 13; noiseState ^= noiseState >> 17; noiseState ^= noiseState << 5;
            const float noise = ((float) (noiseState & 0xFFFFu) / 32768.0f - 1.0f) * clickNoise;

            clickPhase += clickIncrement;
            const float tone = (float) std::sin (clickPhase);

            const float click = (tone * 0.75f + clickBody.process (noise) * 0.9f) * clickEnv;
            data[n] += click * gain;

            clickEnv *= clickDecay;
            clickNoise *= clickDecay * 0.92f;   // the noise burst dies first
        }
        else
        {
            clickEnv = 0.0f;
        }
    }
}

} // namespace bassamp
