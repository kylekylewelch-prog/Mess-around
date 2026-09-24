#include "bassamp/Oversampler.h"

namespace bassamp {

void Oversampler::prepare (double baseSampleRate, int maxBlockSize, int factor)
{
    baseRate = baseSampleRate;
    oversampleFactor = (factor >= 4 ? 4 : (factor >= 2 ? 2 : 1));
    buffer.assign ((size_t) std::max (1, maxBlockSize) * (size_t) oversampleFactor, 0.0f);

    if (oversampleFactor > 1)
    {
        // Keep everything the bass rig actually uses, kill everything that would
        // fold back. 16 kHz is comfortably above the top of any cabinet response.
        const float cutoff = std::min (16000.0f, (float) baseRate * 0.45f);

        // First stage runs at 2x the base rate, second (only for 4x) at 4x.
        upFilter1.design   (baseRate * 2.0, cutoff);
        downFilter1.design (baseRate * 2.0, cutoff);
        if (oversampleFactor == 4)
        {
            upFilter2.design   (baseRate * 4.0, cutoff);
            downFilter2.design (baseRate * 4.0, cutoff);
        }
    }
    reset();
}

void Oversampler::reset()
{
    upFilter1.reset(); upFilter2.reset();
    downFilter1.reset(); downFilter2.reset();
    std::fill (buffer.begin(), buffer.end(), 0.0f);
}

float* Oversampler::upsample (const float* input, int numSamples)
{
    if ((size_t) (numSamples * oversampleFactor) > buffer.size())
        buffer.assign ((size_t) numSamples * (size_t) oversampleFactor, 0.0f);

    if (oversampleFactor == 1)
    {
        std::copy (input, input + numSamples, buffer.begin());
        return buffer.data();
    }

    // Zero stuffing loses 1/factor of the amplitude, so compensate as we insert.
    const float gain = (float) oversampleFactor;

    if (oversampleFactor == 2)
    {
        for (int n = 0; n < numSamples; ++n)
        {
            buffer[(size_t) (2 * n)]     = upFilter1.process (input[n] * gain);
            buffer[(size_t) (2 * n + 1)] = upFilter1.process (0.0f);
        }
    }
    else
    {
        for (int n = 0; n < numSamples; ++n)
        {
            // 1 -> 2
            const float a = upFilter1.process (input[n] * 2.0f);
            const float b = upFilter1.process (0.0f);
            // 2 -> 4
            buffer[(size_t) (4 * n)]     = upFilter2.process (a * 2.0f);
            buffer[(size_t) (4 * n + 1)] = upFilter2.process (0.0f);
            buffer[(size_t) (4 * n + 2)] = upFilter2.process (b * 2.0f);
            buffer[(size_t) (4 * n + 3)] = upFilter2.process (0.0f);
        }
    }
    return buffer.data();
}

void Oversampler::downsample (float* output, int numSamples)
{
    if (oversampleFactor == 1)
    {
        std::copy (buffer.begin(), buffer.begin() + numSamples, output);
        return;
    }

    if (oversampleFactor == 2)
    {
        for (int n = 0; n < numSamples; ++n)
        {
            const float a = downFilter1.process (buffer[(size_t) (2 * n)]);
            downFilter1.process (buffer[(size_t) (2 * n + 1)]);
            output[n] = a;
        }
    }
    else
    {
        for (int n = 0; n < numSamples; ++n)
        {
            // 4 -> 2
            const float a = downFilter2.process (buffer[(size_t) (4 * n)]);
            downFilter2.process (buffer[(size_t) (4 * n + 1)]);
            const float b = downFilter2.process (buffer[(size_t) (4 * n + 2)]);
            downFilter2.process (buffer[(size_t) (4 * n + 3)]);
            // 2 -> 1
            const float c = downFilter1.process (a);
            downFilter1.process (b);
            output[n] = c;
        }
    }
}

} // namespace bassamp
