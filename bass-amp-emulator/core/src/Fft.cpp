#include "bassamp/Fft.h"

namespace bassamp {

void Fft::setOrder (int newOrder)
{
    if (newOrder == order && size > 0) return;

    order = std::max (1, newOrder);
    size  = 1 << order;

    bitReverse.resize ((size_t) size);
    for (int i = 0; i < size; ++i)
    {
        int rev = 0;
        for (int b = 0; b < order; ++b)
            if (i & (1 << b)) rev |= 1 << (order - 1 - b);
        bitReverse[(size_t) i] = rev;
    }

    // Twiddles for every stage, laid out stage by stage so the inner loop walks
    // memory linearly.
    twiddleRe.clear();
    twiddleIm.clear();
    twiddleRe.reserve ((size_t) size);
    twiddleIm.reserve ((size_t) size);
    for (int len = 2; len <= size; len <<= 1)
    {
        const double ang = -2.0 * kPiD / (double) len;
        for (int j = 0; j < len / 2; ++j)
        {
            twiddleRe.push_back ((float) std::cos (ang * j));
            twiddleIm.push_back ((float) std::sin (ang * j));
        }
    }
}

void Fft::transform (float* re, float* im, bool inverseTransform) const
{
    // Decimation in time: permute into bit-reversed order first.
    for (int i = 0; i < size; ++i)
    {
        const int j = bitReverse[(size_t) i];
        if (j > i) { std::swap (re[i], re[j]); std::swap (im[i], im[j]); }
    }

    int twiddleOffset = 0;
    for (int len = 2; len <= size; len <<= 1)
    {
        const int half = len / 2;
        for (int i = 0; i < size; i += len)
        {
            for (int j = 0; j < half; ++j)
            {
                const float wr = twiddleRe[(size_t) (twiddleOffset + j)];
                const float wi = inverseTransform ? -twiddleIm[(size_t) (twiddleOffset + j)]
                                                  :  twiddleIm[(size_t) (twiddleOffset + j)];
                const int a = i + j, b = i + j + half;
                const float tr = re[b] * wr - im[b] * wi;
                const float ti = re[b] * wi + im[b] * wr;
                re[b] = re[a] - tr; im[b] = im[a] - ti;
                re[a] += tr;        im[a] += ti;
            }
        }
        twiddleOffset += half;
    }

    if (inverseTransform)
    {
        const float scale = 1.0f / (float) size;
        for (int i = 0; i < size; ++i) { re[i] *= scale; im[i] *= scale; }
    }
}

void Fft::forwardReal (const float* input, float* re, float* im) const
{
    for (int i = 0; i < size; ++i) { re[i] = input[i]; im[i] = 0.0f; }
    forward (re, im);
}

} // namespace bassamp
