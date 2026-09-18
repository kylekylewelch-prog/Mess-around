#include "bassamp/IrDesigner.h"
#include <complex>

namespace bassamp {

static Biquad makeBiquad (const FilterSpec& s, double fs)
{
    Biquad b;
    switch (s.type)
    {
        case FilterSpec::Type::LowShelf:  b.setLowShelf  (fs, s.freq, s.q, s.gainDb); break;
        case FilterSpec::Type::HighShelf: b.setHighShelf (fs, s.freq, s.q, s.gainDb); break;
        case FilterSpec::Type::Peak:      b.setPeaking   (fs, s.freq, s.q, s.gainDb); break;
        case FilterSpec::Type::LowPass:   b.setLowpass   (fs, s.freq, s.q);           break;
        case FilterSpec::Type::HighPass:  b.setHighpass  (fs, s.freq, s.q);           break;
        case FilterSpec::Type::Notch:     b.setNotch     (fs, s.freq, s.q);           break;
    }
    return b;
}

float IrDesigner::magnitudeOf (const std::vector<FilterSpec>& specs, float freq, double fs)
{
    float mag = 1.0f;
    for (const auto& s : specs)
    {
        const Biquad b = makeBiquad (s, fs);
        const float m = b.magnitudeAt (freq, fs);
        for (int r = 0; r < std::max (1, s.repeats); ++r) mag *= m;
    }
    return mag;
}

void IrDesigner::designMinimumPhase (const std::vector<FilterSpec>& specs,
                                     double fs, int irLength, std::vector<float>& out)
{
    // Work at 8x the IR length so the cepstral fold has room; aliasing in the
    // cepstrum shows up as a smeared, non-causal tail otherwise.
    const int n = nextPowerOfTwo (std::max (1024, irLength * 8));
    Fft fft (log2Int (n));

    std::vector<float> re ((size_t) n, 0.0f), im ((size_t) n, 0.0f);

    // Precompute the biquad set once; magnitudeOf() would rebuild it per bin.
    std::vector<Biquad> sections;
    std::vector<int>    repeats;
    sections.reserve (specs.size());
    for (const auto& s : specs) { sections.push_back (makeBiquad (s, fs)); repeats.push_back (std::max (1, s.repeats)); }

    for (int k = 0; k <= n / 2; ++k)
    {
        const float freq = (float) k * (float) fs / (float) n;
        float mag = 1.0f;
        for (size_t i = 0; i < sections.size(); ++i)
        {
            const float m = sections[i].magnitudeAt (std::max (freq, 0.5f), fs);
            for (int r = 0; r < repeats[i]; ++r) mag *= m;
        }
        // Floor the magnitude: log(0) is -inf and the cepstrum would blow up.
        const float logMag = std::log (std::max (mag, 1.0e-7f));
        re[(size_t) k] = logMag;
        if (k > 0 && k < n / 2) re[(size_t) (n - k)] = logMag;
    }

    // Real cepstrum.
    std::fill (im.begin(), im.end(), 0.0f);
    fft.inverse (re.data(), im.data());

    // Fold the anticausal half onto the causal half: this is what turns an
    // arbitrary magnitude response into its minimum-phase counterpart.
    std::vector<float> cre ((size_t) n, 0.0f), cim ((size_t) n, 0.0f);
    cre[0] = re[0];
    for (int k = 1; k < n / 2; ++k) cre[(size_t) k] = 2.0f * re[(size_t) k];
    cre[(size_t) (n / 2)] = re[(size_t) (n / 2)];

    fft.forward (cre.data(), cim.data());

    // exp() of the complex log spectrum gives the minimum-phase spectrum.
    for (int k = 0; k < n; ++k)
    {
        const float mag = std::exp (cre[(size_t) k]);
        const float ph  = cim[(size_t) k];
        cre[(size_t) k] = mag * std::cos (ph);
        cim[(size_t) k] = mag * std::sin (ph);
    }

    fft.inverse (cre.data(), cim.data());

    out.assign ((size_t) irLength, 0.0f);
    const int fadeStart = (irLength * 3) / 4;
    for (int i = 0; i < irLength; ++i)
    {
        float w = 1.0f;
        if (i >= fadeStart)
        {
            const float t = (float) (i - fadeStart) / (float) std::max (1, irLength - fadeStart);
            w = 0.5f * (1.0f + std::cos (kPi * t));   // raised cosine tail
        }
        out[(size_t) i] = cre[(size_t) i] * w;
    }
}

void IrDesigner::normaliseAt (std::vector<float>& ir, double fs, float refFreq)
{
    if (ir.empty()) return;

    // Measure the steady-state response at the reference frequency by running a
    // sine through the IR analytically (DFT at that single bin).
    double sre = 0.0, sim = 0.0;
    const double w = 2.0 * kPiD * refFreq / fs;
    for (size_t i = 0; i < ir.size(); ++i)
    {
        sre += ir[i] * std::cos (w * (double) i);
        sim -= ir[i] * std::sin (w * (double) i);
    }
    const double mag = std::sqrt (sre * sre + sim * sim);
    if (mag < 1.0e-9) return;

    const float scale = (float) (1.0 / mag);
    for (auto& v : ir) v *= scale;
}

} // namespace bassamp
