#include "bassamp/Convolver.h"

namespace bassamp {

void Convolver::prepare (double, int, int partitionSize)
{
    partition  = std::max (32, nextPowerOfTwo (partitionSize));
    headLength = partition;
    fftSize    = partition * 2;
    fft.setOrder (log2Int (fftSize));

    head.assign ((size_t) headLength, 0.0f);
    headDelay.assign ((size_t) headLength * 2, 0.0f);
    inputBlock.assign ((size_t) partition, 0.0f);
    overlap.assign ((size_t) partition, 0.0f);
    tailOut.assign ((size_t) partition, 0.0f);
    workRe.assign ((size_t) fftSize, 0.0f);
    workIm.assign ((size_t) fftSize, 0.0f);
    accRe.assign  ((size_t) fftSize, 0.0f);
    accIm.assign  ((size_t) fftSize, 0.0f);

    reset();
}

void Convolver::reset()
{
    std::fill (headDelay.begin(), headDelay.end(), 0.0f);
    std::fill (inputBlock.begin(), inputBlock.end(), 0.0f);
    std::fill (overlap.begin(), overlap.end(), 0.0f);
    std::fill (tailOut.begin(), tailOut.end(), 0.0f);
    std::fill (fdlRe.begin(), fdlRe.end(), 0.0f);
    std::fill (fdlIm.begin(), fdlIm.end(), 0.0f);
    headWritePos = 0;
    inputFill = 0;
    tailRead = partition;   // nothing pending yet
    fdlIndex = 0;
}

void Convolver::setImpulseResponse (const float* ir, int numSamples)
{
    if (head.empty()) prepare (48000.0, 512, partition);

    irLength = std::max (0, numSamples);
    std::fill (head.begin(), head.end(), 0.0f);

    const int headTaps = std::min (irLength, headLength);
    for (int i = 0; i < headTaps; ++i) head[(size_t) i] = ir[i];

    const int tailTaps = std::max (0, irLength - headLength);
    numPartitions = (tailTaps + partition - 1) / partition;

    filterRe.assign ((size_t) std::max (1, numPartitions) * (size_t) fftSize, 0.0f);
    filterIm.assign ((size_t) std::max (1, numPartitions) * (size_t) fftSize, 0.0f);
    fdlRe.assign    ((size_t) std::max (1, numPartitions) * (size_t) fftSize, 0.0f);
    fdlIm.assign    ((size_t) std::max (1, numPartitions) * (size_t) fftSize, 0.0f);

    for (int p = 0; p < numPartitions; ++p)
    {
        std::fill (workRe.begin(), workRe.end(), 0.0f);
        std::fill (workIm.begin(), workIm.end(), 0.0f);
        // Partition p covers IR taps [headLength + p*partition, ...). The taps go
        // in the first half of the FFT frame; the second half stays zero so the
        // circular convolution behaves like a linear one.
        for (int i = 0; i < partition; ++i)
        {
            const int idx = headLength + p * partition + i;
            if (idx < irLength) workRe[(size_t) i] = ir[idx];
        }
        fft.forward (workRe.data(), workIm.data());
        std::copy (workRe.begin(), workRe.end(), filterRe.begin() + (size_t) p * fftSize);
        std::copy (workIm.begin(), workIm.end(), filterIm.begin() + (size_t) p * fftSize);
    }

    reset();
}

// Called once per completed input partition. Transforms the newest block, stores
// it in the frequency-domain delay line, multiply-accumulates against every
// filter partition and inverse-transforms into the pending tail buffer.
void Convolver::pushFftBlock()
{
    if (numPartitions <= 0)
    {
        std::fill (tailOut.begin(), tailOut.end(), 0.0f);
        tailRead = 0;
        return;
    }

    // Overlap-save frame: previous partition followed by the current one.
    for (int i = 0; i < partition; ++i)
    {
        workRe[(size_t) i] = overlap[(size_t) i];
        workIm[(size_t) i] = 0.0f;
        workRe[(size_t) (partition + i)] = inputBlock[(size_t) i];
        workIm[(size_t) (partition + i)] = 0.0f;
    }
    fft.forward (workRe.data(), workIm.data());

    std::copy (workRe.begin(), workRe.end(), fdlRe.begin() + (size_t) fdlIndex * fftSize);
    std::copy (workIm.begin(), workIm.end(), fdlIm.begin() + (size_t) fdlIndex * fftSize);

    std::fill (accRe.begin(), accRe.end(), 0.0f);
    std::fill (accIm.begin(), accIm.end(), 0.0f);

    for (int p = 0; p < numPartitions; ++p)
    {
        // Filter partition p pairs with the input block from p frames ago.
        int slot = fdlIndex - p;
        while (slot < 0) slot += numPartitions;

        const float* xr = fdlRe.data()    + (size_t) slot * fftSize;
        const float* xi = fdlIm.data()    + (size_t) slot * fftSize;
        const float* hr = filterRe.data() + (size_t) p    * fftSize;
        const float* hi = filterIm.data() + (size_t) p    * fftSize;

        for (int k = 0; k < fftSize; ++k)
        {
            accRe[(size_t) k] += xr[k] * hr[k] - xi[k] * hi[k];
            accIm[(size_t) k] += xr[k] * hi[k] + xi[k] * hr[k];
        }
    }

    fft.inverse (accRe.data(), accIm.data());

    // Overlap-save: the valid linear-convolution output is the second half.
    for (int i = 0; i < partition; ++i)
        tailOut[(size_t) i] = accRe[(size_t) (partition + i)];

    tailRead = 0;
    std::copy (inputBlock.begin(), inputBlock.end(), overlap.begin());
    fdlIndex = (fdlIndex + 1) % numPartitions;
}

void Convolver::process (float* data, int numSamples)
{
    if (irLength <= 0) return;

    const int headMask = headLength * 2 - 1;

    for (int n = 0; n < numSamples; ++n)
    {
        const float x = data[n];

        headDelay[(size_t) headWritePos] = x;

        // Direct FIR over the first headLength taps.
        float y = 0.0f;
        for (int k = 0; k < headLength; ++k)
            y += head[(size_t) k] * headDelay[(size_t) ((headWritePos - k) & headMask)];

        headWritePos = (headWritePos + 1) & headMask;

        // FFT section, one partition behind by construction.
        if (tailRead < partition) y += tailOut[(size_t) tailRead++];

        inputBlock[(size_t) inputFill++] = x;
        if (inputFill == partition) { inputFill = 0; pushFftBlock(); }

        data[n] = y;
    }
}

} // namespace bassamp
