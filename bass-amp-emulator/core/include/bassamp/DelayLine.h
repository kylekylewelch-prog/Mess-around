// DelayLine.h - fractional delay line for the modulation and delay pedals.
#pragma once

#include "Common.h"
#include <vector>

namespace bassamp {

class DelayLine
{
public:
    void prepare (double sampleRate, float maxDelaySeconds)
    {
        fs = sampleRate;
        const int len = nextPow2 ((int) (maxDelaySeconds * sampleRate) + 4);
        buffer.assign ((size_t) len, 0.0f);
        mask = len - 1;
        writePos = 0;
    }

    void reset()
    {
        std::fill (buffer.begin(), buffer.end(), 0.0f);
        writePos = 0;
    }

    inline void write (float x) { buffer[(size_t) writePos] = x; writePos = (writePos + 1) & mask; }

    // Cubic (Catmull-Rom) interpolated read. Smooth enough that a modulated
    // delay does not produce the zipper artefacts linear interpolation gives you
    // on a slow flanger sweep.
    inline float read (float delaySamples) const
    {
        const float d = clampf (delaySamples, 1.0f, (float) (mask - 2));
        const int   i = (int) d;
        const float f = d - (float) i;

        // write() has already advanced writePos, so the most recent sample sits
        // at writePos - 1. p1 is the sample at delay `i`, p2/p3 are older, p0 newer.
        const int len = mask + 1;
        const int i0 = (writePos - 1 - i + 2 * len) & mask;
        const int i1 = (i0 - 1 + len) & mask;
        const int i2 = (i0 - 2 + 2 * len) & mask;
        const int im = (i0 + 1) & mask;

        const float p0 = buffer[(size_t) im], p1 = buffer[(size_t) i0];
        const float p2 = buffer[(size_t) i1], p3 = buffer[(size_t) i2];

        return 0.5f * ((2.0f * p1)
                     + (-p0 + p2) * f
                     + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * f * f
                     + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * f * f * f);
    }

    int getMaxDelaySamples() const { return mask - 2; }

private:
    static int nextPow2 (int n) { int p = 2; while (p < n) p <<= 1; return p; }

    std::vector<float> buffer;
    int mask = 0, writePos = 0;
    double fs = 48000.0;
};

// Low-frequency oscillator with the waveshapes the modulation pedals need.
struct Lfo
{
    enum class Shape { Sine, Triangle, Square, Ramp, RandomStep };

    void prepare (double sampleRate) { fs = sampleRate; phase = 0.0f; randomValue = 0.0f; }
    void reset (float startPhase = 0.0f) { phase = startPhase; }
    // The upper limit is set by the ring modulator's carrier, not by any LFO use.
    void setRate (float hz) { inc = clampf (hz, 0.01f, 5000.0f) / (float) fs; }

    inline float next (Shape shape)
    {
        const float p = phase;
        phase += inc;
        if (phase >= 1.0f)
        {
            phase -= 1.0f;
            // 32-bit xorshift: deterministic, no allocation, plenty random for a
            // sample-and-hold modulator.
            rngState ^= rngState << 13; rngState ^= rngState >> 17; rngState ^= rngState << 5;
            randomValue = ((float) (rngState & 0xFFFF) / 32768.0f) - 1.0f;
        }

        switch (shape)
        {
            case Shape::Sine:       return std::sin (kTwoPi * p);
            case Shape::Triangle:   return 4.0f * std::fabs (p - 0.5f) - 1.0f;
            case Shape::Square:     return p < 0.5f ? 1.0f : -1.0f;
            case Shape::Ramp:       return 2.0f * p - 1.0f;
            case Shape::RandomStep: return randomValue;
        }
        return 0.0f;
    }

    float getPhase() const { return phase; }

private:
    double fs = 48000.0;
    float phase = 0.0f, inc = 0.001f, randomValue = 0.0f;
    uint32_t rngState = 0x1234567u;
};

// Attack/release envelope follower used by the compressor, the filters and the
// synth. Detects on the rectified signal; RMS mode is closer to how an opto cell
// behaves, peak mode to a FET.
struct EnvelopeFollower
{
    void prepare (double sampleRate) { fs = sampleRate; setTimes (0.005f, 0.100f); env = 0.0f; }
    void setTimes (float attackSec, float releaseSec)
    {
        attack  = timeConstantCoeff (attackSec,  fs);
        release = timeConstantCoeff (releaseSec, fs);
    }
    void reset() { env = 0.0f; }

    inline float process (float x)
    {
        const float rect = std::fabs (x);
        env = flushDenormal (rect > env ? attack  * env + (1.0f - attack)  * rect
                                        : release * env + (1.0f - release) * rect);
        return env;
    }

    float value() const { return env; }

private:
    double fs = 48000.0;
    float env = 0.0f, attack = 0.9f, release = 0.99f;
};

} // namespace bassamp
