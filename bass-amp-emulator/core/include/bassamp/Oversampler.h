// Oversampler.h - anti-aliasing oversampling for the nonlinear stages.
//
// Design note: the usual FIR halfband oversampler has linear phase but costs
// latency - a 63-tap halfband at 2x is another ~0.3 ms, and you pay it twice
// (up and down) on every nonlinear stage in the chain. For a live rig that is a
// bad trade, so this uses a cascaded Butterworth IIR instead. Phase is not
// linear, but the group delay at bass frequencies is a handful of microseconds
// and the stopband rejection at the folding frequency is better than -70 dB,
// which is well below the noise floor of the analog gear being modelled.
#pragma once

#include "Common.h"
#include "Filters.h"
#include <vector>

namespace bassamp {

// Cascade of Butterworth biquad sections (even order only).
template <int Order>
struct ButterworthLowpass
{
    static_assert (Order % 2 == 0, "Butterworth cascade needs an even order");
    static constexpr int NumStages = Order / 2;
    Biquad stages[NumStages];

    void design (double fs, float cutoffHz)
    {
        for (int k = 0; k < NumStages; ++k)
        {
            // Butterworth pole Qs: 1 / (2 cos(pi (2k+1) / (2n)))
            const float q = 1.0f / (2.0f * std::cos (kPi * (float) (2 * k + 1) / (float) (2 * Order)));
            stages[k].setLowpass (fs, cutoffHz, q);
        }
    }

    void reset() { for (auto& s : stages) s.reset(); }

    inline float process (float x)
    {
        for (auto& s : stages) x = s.process (x);
        return x;
    }
};

class Oversampler
{
public:
    // factor must be 1, 2 or 4. factor == 1 is a transparent pass-through so the
    // caller can offer an "off" setting without branching in the audio loop.
    void prepare (double baseSampleRate, int maxBlockSize, int factor);

    void reset();

    int    getFactor()           const { return oversampleFactor; }
    double getOversampledRate()  const { return baseRate * oversampleFactor; }
    int    getLatencySamples()   const { return 0; } // IIR path: no integer delay

    // Expands numSamples of `input` into the internal oversampled buffer and
    // returns a pointer to it (numSamples * factor valid samples).
    float* upsample (const float* input, int numSamples);

    // Filters and decimates the internal buffer back into `output`.
    void downsample (float* output, int numSamples);

private:
    int    oversampleFactor = 1;
    double baseRate = 48000.0;
    std::vector<float> buffer;

    ButterworthLowpass<8> upFilter1, upFilter2, downFilter1, downFilter2;
};

} // namespace bassamp
