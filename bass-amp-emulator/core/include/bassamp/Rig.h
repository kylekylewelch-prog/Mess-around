// Rig.h - the complete signal chain, from input jack to monitor output.
//
// Signal flow:
//
//   input -> gain -> [tuner tap] -> pedals (front of amp) -> AMP
//         -> pedals (amp loop) -> CABINET -> DI blend -> pedals (post cab)
//         -> master -> output (+ metronome, + backing track)
//
// The tuner runs off a tap rather than in line, so it never adds anything to the
// signal path, and the metronome and backing track are mixed in after the master
// so they are monitored but never processed by the rig.
#pragma once

#include "Amp.h"
#include "Cabinet.h"
#include "Metronome.h"
#include "Pedalboard.h"
#include "Tuner.h"
#include <string>

namespace bassamp {

struct RigSettings
{
    float inputGainDb   = 0.0f;
    float masterLevelDb = -3.0f;
    float diBlend       = 0.0f;    // 0 = all cab, 1 = all direct
    bool  muteWhileTuning = false;
    bool  tunerActive   = false;
    int   oversampling  = 2;       // 1 (off), 2 (live), 4 (studio)
};

// Playback helper for the track you are playing along with. Kept in the core so
// the "cut the recorded bass" filter is testable.
struct BackingTrackProcessor
{
    void prepare (double sampleRate);
    void reset();
    // In place, stereo. `bassCut` removes centre-panned low content so the
    // recorded bass part gets out of the way of yours.
    void process (float* left, float* right, int numSamples, float gain, float bassCut);

private:
    Biquad sideHp, midLowShelf;
};

class Rig
{
public:
    Rig();

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    // Mono in (the bass), stereo out (what you monitor).
    void process (const float* input, float* outLeft, float* outRight, int numSamples);

    // --- sub-sections -------------------------------------------------------
    Amp&        amp()        { return ampSection; }
    Cabinet&    cabinet()    { return cabSection; }
    Pedalboard& pedalboard() { return board; }
    Tuner&      tuner()      { return tunerSection; }
    Metronome&  metronome()  { return metronomeSection; }

    const Amp&        amp()        const { return ampSection; }
    const Cabinet&    cabinet()    const { return cabSection; }
    const Pedalboard& pedalboard() const { return board; }

    // --- settings -----------------------------------------------------------
    void setSettings (const RigSettings& s);
    const RigSettings& getSettings() const { return settings; }

    // Cabinet settings are applied from the message thread; the rig retries
    // automatically if a previous swap is still in flight.
    void requestCabinetSettings (const CabinetSettings& s);
    const CabinetSettings& getCabinetSettings() const { return pendingCab; }
    void serviceMessageThread();   // call from a timer: applies deferred work

    // --- metering -----------------------------------------------------------
    float getInputLevel()  const { return inputMeter.getPeak(); }
    float getOutputLevel() const { return outputMeter.getPeak(); }
    bool  isInputClipping() const { return inputMeter.isClipping(); }
    float getLatencySamples() const { return 0.0f; }   // the rig itself adds none

    // --- presets ------------------------------------------------------------
    std::string saveState() const;
    bool loadState (const std::string& text);

private:
    double fs = 48000.0;
    int    maxBlock = 512;

    RigSettings settings;
    CabinetSettings pendingCab, appliedCab;
    bool cabDirty = false;

    Amp        ampSection;
    Cabinet    cabSection;
    Pedalboard board;
    Tuner      tunerSection;
    Metronome  metronomeSection;

    SmoothedValue inputGain, masterGain, diBlendSmooth, tunerMute;
    LevelMeter inputMeter, outputMeter;

    std::vector<float> work, direct;
};

} // namespace bassamp
