// Metronome.h - sample-accurate click with full time-signature support.
//
// The scheduler counts in samples, not blocks, so the click lands on the right
// sample regardless of buffer size and never drifts - a metronome that wanders
// by half a millisecond every bar is worse than no metronome at all. The same
// clock drives tempo sync for the delay and tremolo.
#pragma once

#include "Common.h"
#include "Filters.h"
#include <atomic>
#include <vector>

namespace bassamp {

struct TimeSignature
{
    int numerator = 4;
    int denominator = 4;   // 1, 2, 4, 8 or 16

    bool isCompound() const
    {
        // 6/8, 9/8 and 12/8 are felt in dotted-quarter pulses, not in eighths.
        return denominator == 8 && (numerator == 6 || numerator == 9 || numerator == 12);
    }
};

enum class Subdivision
{
    Quarter = 0,
    Eighth,
    Triplet,
    Sixteenth,
    NumSubdivisions
};

const char* subdivisionName (Subdivision s);

enum class AccentLevel { Silent = 0, Normal, Accent };

class Metronome
{
public:
    static constexpr int kMaxBeats = 16;

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    // --- audio thread -------------------------------------------------------
    // Mixes the click into `data` (the click is monitored, not processed by the rig).
    void process (float* data, int numSamples);

    // --- UI thread ----------------------------------------------------------
    void setRunning (bool shouldRun);
    bool isRunning() const { return running.load(); }

    void setTempo (double bpm);
    double getTempo() const { return tempoBpm.load(); }

    void setTimeSignature (int numerator, int denominator);
    TimeSignature getTimeSignature() const;

    void setSubdivision (Subdivision s) { subdivision.store ((int) s); }
    Subdivision getSubdivision() const  { return (Subdivision) subdivision.load(); }

    void setSwing (float amount) { swing.store (clampf (amount, 0.0f, 0.75f)); }
    float getSwing() const       { return swing.load(); }

    void setLevel (float gain) { level.store (clampf (gain, 0.0f, 2.0f)); }
    float getLevel() const     { return level.load(); }

    void setCountInBars (int bars) { countInBars.store (std::max (0, std::min (bars, 4))); }
    int  getCountInBars() const    { return countInBars.load(); }

    void setAccent (int beatIndex, AccentLevel a);
    AccentLevel getAccent (int beatIndex) const;

    // Tap tempo. Call on each tap with the host's current time in seconds.
    void tap (double timeSeconds);

    // For the beat display and for pedals that follow the clock.
    int   getCurrentBeat() const  { return currentBeat.load(); }
    int   getBeatsPerBar() const;
    float getBeatProgress() const { return beatProgress.load(); }
    bool  isCountingIn() const    { return countInRemaining.load() > 0; }

private:
    void recalculate();
    void triggerClick (AccentLevel accentLevel, bool isSubdivision);

    double fs = 48000.0;

    std::atomic<bool>   running { false };
    std::atomic<double> tempoBpm { 120.0 };
    std::atomic<int>    numerator { 4 }, denominator { 4 };
    std::atomic<int>    subdivision { (int) Subdivision::Quarter };
    std::atomic<float>  swing { 0.0f }, level { 0.6f };
    std::atomic<int>    countInBars { 0 }, countInRemaining { 0 };
    std::atomic<int>    currentBeat { 0 };
    std::atomic<float>  beatProgress { 0.0f };
    std::atomic<int>    accents[kMaxBeats];
    std::atomic<bool>   restartRequested { true };

    // Scheduler state (audio thread)
    double samplesPerTick = 24000.0;
    double tickPhase = 0.0;
    int    ticksPerBeat = 1;
    int    tickInBeat = 0;
    int    beatInBar = 0;
    bool   firstTick = true;

    // Click voice
    float  clickEnv = 0.0f, clickDecay = 0.999f;
    double clickPhase = 0.0, clickIncrement = 0.0;
    float  clickNoise = 0.0f;
    uint32_t noiseState = 0x2545F491u;
    Biquad clickBody;

    std::vector<double> tapTimes;
};

} // namespace bassamp
