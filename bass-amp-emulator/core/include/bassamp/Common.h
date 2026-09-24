// Common.h - shared numeric helpers for the BassAmp DSP core.
//
// The core is deliberately free of external dependencies: it compiles with a
// plain C++17 toolchain so the DSP can be unit tested away from the audio host.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace bassamp {

constexpr float  kPi        = 3.14159265358979323846f;
constexpr float  kTwoPi     = 6.28318530717958647692f;
constexpr double kPiD       = 3.14159265358979323846;
constexpr float  kSqrt2Over2 = 0.70710678118654752440f;

// Anything quieter than this is treated as silence by the envelope followers and
// gates. -120 dBFS is far below the noise floor of any real bass rig.
constexpr float kSilenceGain = 1.0e-6f;

inline float dbToGain (float db)          { return std::pow (10.0f, db * 0.05f); }
inline float gainToDb (float gain)        { return 20.0f * std::log10 (std::max (gain, kSilenceGain)); }
inline float clampf   (float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline float lerp     (float a, float b, float t)   { return a + (b - a) * t; }

// Maps a normalised 0..1 control to a range with a skew: skew = 0.5 is linear,
// smaller values push resolution towards the bottom of the range (what you want
// for frequency and time controls).
inline float skewedRange (float norm, float lo, float hi, float skew)
{
    const float t = std::pow (clampf (norm, 0.0f, 1.0f), std::log (0.5f) / std::log (clampf (skew, 0.01f, 0.99f)));
    return lo + (hi - lo) * t;
}

inline float logRange (float norm, float lo, float hi)
{
    return lo * std::pow (hi / lo, clampf (norm, 0.0f, 1.0f));
}

// Denormals cost hundreds of cycles on x86 when reverb/delay tails decay below
// ~1e-38. We flush rather than rely on the host setting FTZ/DAZ.
inline float flushDenormal (float v)
{
    return (std::fabs (v) < 1.0e-30f) ? 0.0f : v;
}

inline bool isFinite (float v)
{
    return std::isfinite (v);
}

// Cheap tanh approximation. Max error ~2e-4 over +/-4, monotonic, and it costs a
// division instead of an exp - meaningful when it runs inside a 4x oversampled
// tube stage on every sample.
inline float fastTanh (float x)
{
    if (x < -4.5f) return -1.0f;
    if (x >  4.5f) return  1.0f;
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

// One-pole coefficient for a given time constant, i.e. the per-sample decay of
// exp(-1/(tau*fs)). Used by every envelope follower and the power-supply sag model.
inline float timeConstantCoeff (float timeSeconds, double sampleRate)
{
    if (timeSeconds <= 0.0f) return 0.0f;
    return std::exp (-1.0f / (float) (timeSeconds * sampleRate));
}

// Linear parameter smoother. Audio-thread safe: setTarget() is called from the
// block boundary, next() runs per sample.
class SmoothedValue
{
public:
    void reset (double sampleRate, float rampSeconds, float initial = 0.0f)
    {
        rampSamples = std::max (1, (int) (rampSeconds * sampleRate));
        current = target = initial;
        step = 0.0f;
        countdown = 0;
    }

    void setTarget (float newTarget)
    {
        if (newTarget == target) return;
        target = newTarget;
        step = (target - current) / (float) rampSamples;
        countdown = rampSamples;
    }

    void snap (float value) { current = target = value; countdown = 0; step = 0.0f; }

    inline float next()
    {
        if (countdown > 0) { current += step; if (--countdown == 0) current = target; }
        return current;
    }

    float value()  const { return current; }
    float getTarget() const { return target; }
    bool  isSmoothing() const { return countdown > 0; }

private:
    float current = 0.0f, target = 0.0f, step = 0.0f;
    int   rampSamples = 64, countdown = 0;
};

// Peak/RMS meter with a fast attack and a ballistic release, used for the input
// meter and the clip LEDs.
class LevelMeter
{
public:
    void reset (double sampleRate)
    {
        releaseCoeff = timeConstantCoeff (0.3f, sampleRate);
        peak = 0.0f;
        clipHold = 0;
        holdSamples = (int) (sampleRate * 0.8);
    }

    inline void push (float x)
    {
        const float a = std::fabs (x);
        if (a > peak) peak = a;
        else          peak *= releaseCoeff;
        if (a >= 0.99f) clipHold = holdSamples;
        else if (clipHold > 0) --clipHold;
    }

    float getPeak() const   { return peak; }
    bool  isClipping() const { return clipHold > 0; }

private:
    float peak = 0.0f, releaseCoeff = 0.999f;
    int   clipHold = 0, holdSamples = 44100;
};

} // namespace bassamp
