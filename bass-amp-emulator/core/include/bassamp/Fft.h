// Fft.h - small self-contained radix-2 FFT.
//
// Used by the partitioned convolution engine and by the cabinet IR designer.
// Nothing here runs per-sample on the audio thread except the convolver's
// transforms, so clarity beats micro-optimisation - but the twiddles and the
// bit-reversal permutation are precomputed because those do run in the block loop.
#pragma once

#include "Common.h"
#include <vector>

namespace bassamp {

class Fft
{
public:
    Fft() = default;
    explicit Fft (int order) { setOrder (order); }

    // order is log2(size); size must be a power of two.
    void setOrder (int newOrder);

    int getSize()  const { return size; }
    int getOrder() const { return order; }

    // In-place complex transform on split real/imaginary buffers of getSize().
    void forward (float* re, float* im) const { transform (re, im, false); }

    // Inverse transform, scaled by 1/N.
    void inverse (float* re, float* im) const { transform (re, im, true); }

    // Convenience: real input -> complex spectrum (caller supplies size-length buffers).
    void forwardReal (const float* input, float* re, float* im) const;

private:
    void transform (float* re, float* im, bool inverseTransform) const;

    int order = 0, size = 0;
    std::vector<int>   bitReverse;
    std::vector<float> twiddleRe, twiddleIm;
};

// Next power of two >= n.
inline int nextPowerOfTwo (int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

inline int log2Int (int n)
{
    int k = 0;
    while ((1 << k) < n) ++k;
    return k;
}

} // namespace bassamp
