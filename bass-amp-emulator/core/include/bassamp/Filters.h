// Filters.h - the filter primitives every amp, cab and pedal in the rig is built from.
#pragma once

#include "Common.h"
#include <complex>

namespace bassamp {

// Transposed Direct Form II biquad. TDF2 is the right choice here: it has the
// best numerical behaviour of the direct forms at the low centre frequencies a
// bass rig lives at (30-200 Hz), where DF1 coefficient quantisation starts to
// bite at 32-bit.
struct Biquad
{
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f, z2 = 0.0f;

    void reset() { z1 = z2 = 0.0f; }

    inline float process (float x)
    {
        const float y = b0 * x + z1;
        z1 = flushDenormal (b1 * x - a1 * y + z2);
        z2 = flushDenormal (b2 * x - a2 * y);
        return y;
    }

    void setCoefficients (float nb0, float nb1, float nb2, float na0, float na1, float na2)
    {
        const float inv = 1.0f / na0;
        b0 = nb0 * inv; b1 = nb1 * inv; b2 = nb2 * inv;
        a1 = na1 * inv; a2 = na2 * inv;
    }

    // Magnitude response at frequency f (Hz) - used by the cabinet designer and
    // by the response-curve display in the UI.
    float magnitudeAt (float freq, double sampleRate) const
    {
        const double w = 2.0 * kPiD * freq / sampleRate;
        const std::complex<double> z (std::cos (-w), std::sin (-w));
        const std::complex<double> z2c = z * z;
        const std::complex<double> num = (double) b0 + (double) b1 * z + (double) b2 * z2c;
        const std::complex<double> den = 1.0    + (double) a1 * z + (double) a2 * z2c;
        return (float) std::abs (num / den);
    }

    // --- RBJ cookbook designers -------------------------------------------------
    void setLowpass (double fs, float freq, float q)
    {
        const float w0 = kTwoPi * clampf (freq, 1.0f, (float) fs * 0.49f) / (float) fs;
        const float cw = std::cos (w0), sw = std::sin (w0);
        const float alpha = sw / (2.0f * std::max (q, 0.05f));
        setCoefficients ((1 - cw) * 0.5f, 1 - cw, (1 - cw) * 0.5f, 1 + alpha, -2 * cw, 1 - alpha);
    }

    void setHighpass (double fs, float freq, float q)
    {
        const float w0 = kTwoPi * clampf (freq, 1.0f, (float) fs * 0.49f) / (float) fs;
        const float cw = std::cos (w0), sw = std::sin (w0);
        const float alpha = sw / (2.0f * std::max (q, 0.05f));
        setCoefficients ((1 + cw) * 0.5f, -(1 + cw), (1 + cw) * 0.5f, 1 + alpha, -2 * cw, 1 - alpha);
    }

    void setBandpass (double fs, float freq, float q) // constant skirt gain, peak = Q
    {
        const float w0 = kTwoPi * clampf (freq, 1.0f, (float) fs * 0.49f) / (float) fs;
        const float cw = std::cos (w0), sw = std::sin (w0);
        const float alpha = sw / (2.0f * std::max (q, 0.05f));
        setCoefficients (alpha, 0.0f, -alpha, 1 + alpha, -2 * cw, 1 - alpha);
    }

    void setNotch (double fs, float freq, float q)
    {
        const float w0 = kTwoPi * clampf (freq, 1.0f, (float) fs * 0.49f) / (float) fs;
        const float cw = std::cos (w0), sw = std::sin (w0);
        const float alpha = sw / (2.0f * std::max (q, 0.05f));
        setCoefficients (1.0f, -2 * cw, 1.0f, 1 + alpha, -2 * cw, 1 - alpha);
    }

    void setAllpass (double fs, float freq, float q)
    {
        const float w0 = kTwoPi * clampf (freq, 1.0f, (float) fs * 0.49f) / (float) fs;
        const float cw = std::cos (w0), sw = std::sin (w0);
        const float alpha = sw / (2.0f * std::max (q, 0.05f));
        setCoefficients (1 - alpha, -2 * cw, 1 + alpha, 1 + alpha, -2 * cw, 1 - alpha);
    }

    void setPeaking (double fs, float freq, float q, float gainDb)
    {
        const float A  = std::pow (10.0f, gainDb / 40.0f);
        const float w0 = kTwoPi * clampf (freq, 1.0f, (float) fs * 0.49f) / (float) fs;
        const float cw = std::cos (w0), sw = std::sin (w0);
        const float alpha = sw / (2.0f * std::max (q, 0.05f));
        setCoefficients (1 + alpha * A, -2 * cw, 1 - alpha * A, 1 + alpha / A, -2 * cw, 1 - alpha / A);
    }

    void setLowShelf (double fs, float freq, float slope, float gainDb)
    {
        const float A  = std::pow (10.0f, gainDb / 40.0f);
        const float w0 = kTwoPi * clampf (freq, 1.0f, (float) fs * 0.49f) / (float) fs;
        const float cw = std::cos (w0), sw = std::sin (w0);
        const float s  = clampf (slope, 0.1f, 2.0f);
        const float alpha = sw * 0.5f * std::sqrt ((A + 1.0f / A) * (1.0f / s - 1.0f) + 2.0f);
        const float twoSqrtAAlpha = 2.0f * std::sqrt (A) * alpha;
        setCoefficients (A * ((A + 1) - (A - 1) * cw + twoSqrtAAlpha),
                         2 * A * ((A - 1) - (A + 1) * cw),
                         A * ((A + 1) - (A - 1) * cw - twoSqrtAAlpha),
                         (A + 1) + (A - 1) * cw + twoSqrtAAlpha,
                         -2 * ((A - 1) + (A + 1) * cw),
                         (A + 1) + (A - 1) * cw - twoSqrtAAlpha);
    }

    void setHighShelf (double fs, float freq, float slope, float gainDb)
    {
        const float A  = std::pow (10.0f, gainDb / 40.0f);
        const float w0 = kTwoPi * clampf (freq, 1.0f, (float) fs * 0.49f) / (float) fs;
        const float cw = std::cos (w0), sw = std::sin (w0);
        const float s  = clampf (slope, 0.1f, 2.0f);
        const float alpha = sw * 0.5f * std::sqrt ((A + 1.0f / A) * (1.0f / s - 1.0f) + 2.0f);
        const float twoSqrtAAlpha = 2.0f * std::sqrt (A) * alpha;
        setCoefficients (A * ((A + 1) + (A - 1) * cw + twoSqrtAAlpha),
                         -2 * A * ((A - 1) + (A + 1) * cw),
                         A * ((A + 1) + (A - 1) * cw - twoSqrtAAlpha),
                         (A + 1) - (A - 1) * cw + twoSqrtAAlpha,
                         2 * ((A - 1) - (A + 1) * cw),
                         (A + 1) - (A - 1) * cw - twoSqrtAAlpha);
    }
};

// Cascade of N biquads. Cabinet voicings and the graphic EQ are built from these.
template <int N>
struct BiquadChain
{
    Biquad stages[N];

    void reset() { for (auto& s : stages) s.reset(); }

    inline float process (float x)
    {
        for (auto& s : stages) x = s.process (x);
        return x;
    }

    float magnitudeAt (float freq, double fs) const
    {
        float m = 1.0f;
        for (auto& s : stages) m *= s.magnitudeAt (freq, fs);
        return m;
    }
};

// Simple one-pole lowpass. Used for smoothing, envelope shaping and tone caps.
struct OnePole
{
    float a = 0.0f, z = 0.0f;

    void setCutoff (double fs, float freq)
    {
        const float w = kTwoPi * clampf (freq, 0.01f, (float) fs * 0.49f) / (float) fs;
        a = std::exp (-w);
    }
    void setTimeConstant (float seconds, double fs) { a = timeConstantCoeff (seconds, fs); }
    void reset (float v = 0.0f) { z = v; }
    inline float process (float x) { z = flushDenormal (x + a * (z - x)); return z; }
    inline float value() const { return z; }
};

// DC blocker. Every asymmetric clipper in the chain needs one downstream or the
// offset stacks up and eats headroom in the next stage.
struct DCBlocker
{
    float x1 = 0.0f, y1 = 0.0f, R = 0.9995f;

    void prepare (double fs) { R = 1.0f - (kTwoPi * 12.0f / (float) fs); x1 = y1 = 0.0f; }
    void reset() { x1 = y1 = 0.0f; }
    inline float process (float x)
    {
        const float y = x - x1 + R * y1;
        x1 = x; y1 = flushDenormal (y);
        return y;
    }
};

// Topology-preserving-transform state variable filter (Zavalishin). Unlike the
// Chamberlin form it stays stable and correctly tuned right up to Nyquist, which
// matters for the wah and synth sweeps where cutoff is modulated every sample.
struct SvfTPT
{
    float g = 0.1f, k = 1.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
    float ic1eq = 0.0f, ic2eq = 0.0f;
    double fs = 48000.0;

    void prepare (double sampleRate) { fs = sampleRate; setParams (1000.0f, 0.707f); reset(); }
    void reset() { ic1eq = ic2eq = 0.0f; }

    void setParams (float cutoffHz, float q)
    {
        const float f = clampf (cutoffHz, 10.0f, (float) fs * 0.48f);
        g  = std::tan (kPi * f / (float) fs);
        k  = 1.0f / std::max (q, 0.05f);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    struct Out { float lp, bp, hp; };

    inline Out process (float x)
    {
        const float v3 = x - ic2eq;
        const float v1 = a1 * ic1eq + a2 * v3;
        const float v2 = ic2eq + a2 * ic1eq + a3 * v3;
        ic1eq = flushDenormal (2.0f * v1 - ic1eq);
        ic2eq = flushDenormal (2.0f * v2 - ic2eq);
        return { v2, v1, x - k * v1 - v2 };
    }
};

// Linkwitz-Riley 4th-order crossover: two cascaded Butterworth sections per band.
// The magnitude sums flat and both bands stay in phase with each other, which is
// exactly what a bass fuzz needs when you keep the lows clean and only dirty the
// top - a plain pair of filters would comb-filter through the crossover region.
struct LinkwitzRiley4
{
    Biquad lp1, lp2, hp1, hp2;

    void prepare (double fs, float freq)
    {
        lp1.setLowpass  (fs, freq, kSqrt2Over2); lp2 = lp1;
        hp1.setHighpass (fs, freq, kSqrt2Over2); hp2 = hp1;
        reset();
    }
    void reset() { lp1.reset(); lp2.reset(); hp1.reset(); hp2.reset(); }

    inline void process (float x, float& low, float& high)
    {
        low  = lp2.process (lp1.process (x));
        high = hp2.process (hp1.process (x));
    }
};

} // namespace bassamp
