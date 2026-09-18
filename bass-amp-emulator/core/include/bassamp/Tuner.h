// Tuner.h - strobe tuner with YIN pitch detection.
//
// Two independent mechanisms, because they do different jobs:
//
//   * YIN autocorrelation gives the note name and a cents readout. It runs on a
//     background thread over a ~170 ms window, because a low B is 31 Hz and you
//     cannot resolve that from a 5 ms audio block.
//   * The strobe display is a heterodyne: multiply the input by a reference
//     oscillator at the target pitch, low-pass the result, and the phase of what
//     is left rotates at exactly the difference frequency. That is what the
//     spinning disc in a mechanical strobe tuner is showing you, it updates at
//     audio rate, and it resolves to well under a cent - which is why strobe
//     tuners are still what you use for intonation work.
//
// Both run off the same decimated signal, so the cost is a few percent of one
// core even with the tuner open.
#pragma once

#include "Common.h"
#include "Filters.h"
#include <atomic>
#include <string>
#include <vector>

namespace bassamp {

// Single producer (audio thread) / single consumer (analysis thread) FIFO.
class SampleFifo
{
public:
    void prepare (int capacity)
    {
        int size = 2;
        while (size < capacity) size <<= 1;
        buffer.assign ((size_t) size, 0.0f);
        mask = size - 1;
        writeIndex.store (0);
        readIndex.store (0);
    }

    void push (const float* data, int numSamples)
    {
        uint32_t w = writeIndex.load (std::memory_order_relaxed);
        for (int i = 0; i < numSamples; ++i) buffer[(size_t) ((w + (uint32_t) i) & (uint32_t) mask)] = data[i];
        writeIndex.store (w + (uint32_t) numSamples, std::memory_order_release);
    }

    int available() const
    {
        return (int) (writeIndex.load (std::memory_order_acquire) - readIndex.load (std::memory_order_relaxed));
    }

    // Copies the most recent `numSamples` without consuming, and marks everything
    // before them as read. The tuner always wants the newest audio, not a queue.
    bool readLatest (float* dest, int numSamples)
    {
        const uint32_t w = writeIndex.load (std::memory_order_acquire);
        const uint32_t r = readIndex.load (std::memory_order_relaxed);
        if ((int) (w - r) < numSamples) return false;

        const uint32_t start = w - (uint32_t) numSamples;
        for (int i = 0; i < numSamples; ++i) dest[i] = buffer[(size_t) ((start + (uint32_t) i) & (uint32_t) mask)];
        readIndex.store (w, std::memory_order_relaxed);
        return true;
    }

private:
    std::vector<float> buffer;
    int mask = 0;
    std::atomic<uint32_t> writeIndex { 0 }, readIndex { 0 };
};

struct TuningPreset
{
    const char* name;
    int numStrings;
    const char* stringNames[6];
    float midiNotes[6];
};

int                  numTuningPresets();
const TuningPreset&  tuningPreset (int index);

// Note name for a MIDI number, e.g. 28 -> "E1".
std::string midiNoteName (int midiNote);

class Tuner
{
public:
    static constexpr int kStrobeBands = 4;   // fundamental plus three octaves up

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    // --- audio thread -------------------------------------------------------
    void processAudio (const float* data, int numSamples);

    // --- analysis / UI thread ----------------------------------------------
    // Runs one YIN pass if enough audio has accumulated. Cheap to call often.
    void runDetection();

    bool  hasSignal()      const { return signalPresent.load(); }
    float getFrequency()   const { return detectedFrequency.load(); }
    float getCents()       const { return detectedCents.load(); }
    int   getMidiNote()    const { return detectedMidi.load(); }
    float getClarity()     const { return detectedClarity.load(); }
    float getStrobePhase (int band) const;

    void  setReferenceA (float hz) { referenceA.store (clampf (hz, 415.0f, 466.0f)); }
    float getReferenceA() const    { return referenceA.load(); }

    // Locking the tuner to a string frequency is what you want for intonation
    // work; left unlocked it strobes against whatever note it detects.
    void  setTargetMidiNote (int midiNote) { targetMidi.store (midiNote); }
    int   getTargetMidiNote() const        { return targetMidi.load(); }
    void  setAutoTarget (bool shouldAuto)  { autoTarget.store (shouldAuto); }
    bool  isAutoTarget() const             { return autoTarget.load(); }

    float midiToFrequency (float midiNote) const;

private:
    void updateStrobe (float sample);
    float runYin (const float* window, int windowSize, int maxLag, float& clarityOut) const;

    double baseRate = 48000.0, analysisRate = 12000.0;
    int    decimation = 4;

    Biquad decimateLp1, decimateLp2;
    Biquad inputBandpass;
    int    decimateCounter = 0;

    SampleFifo fifo;
    std::vector<float> analysisWindow;
    mutable std::vector<float> yinDifference;

    // Strobe state
    double strobePhase[kStrobeBands] { };
    float  strobeI[kStrobeBands] { }, strobeQ[kStrobeBands] { };
    float  strobeLpCoeff = 0.99f;
    std::atomic<float> strobeAngle[kStrobeBands];
    float  strobeLevel = 0.0f;

    std::atomic<float> detectedFrequency { 0.0f };
    std::atomic<float> detectedCents { 0.0f };
    std::atomic<int>   detectedMidi { 40 };
    std::atomic<float> detectedClarity { 0.0f };
    std::atomic<bool>  signalPresent { false };
    std::atomic<float> referenceA { 440.0f };
    std::atomic<int>   targetMidi { 40 };
    std::atomic<bool>  autoTarget { true };
};

} // namespace bassamp
