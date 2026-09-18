// Validation for the dependency-free DSP primitives.
#include "TestHarness.h"
#include "bassamp/Fft.h"
#include "bassamp/Convolver.h"
#include "bassamp/Oversampler.h"
#include "bassamp/IrDesigner.h"
#include <random>

using namespace bassamp;

static std::vector<float> directConvolve (const std::vector<float>& x, const std::vector<float>& h)
{
    std::vector<float> y (x.size(), 0.0f);
    for (size_t n = 0; n < x.size(); ++n)
        for (size_t k = 0; k < h.size() && k <= n; ++k)
            y[n] += h[k] * x[n - k];
    return y;
}

void testFft()
{
    tst::section ("FFT");
    Fft fft (8); // 256
    std::vector<float> re (256, 0.0f), im (256, 0.0f);

    // A pure bin-8 cosine must produce energy only at bins 8 and 248.
    for (int i = 0; i < 256; ++i) re[(size_t) i] = std::cos (kTwoPi * 8.0f * i / 256.0f);
    fft.forward (re.data(), im.data());
    tst::checkNear (std::hypot (re[8], im[8]), 128.0, 0.05, "FFT peak magnitude at bin 8");
    tst::checkNear (std::hypot (re[20], im[20]), 0.0, 1e-3, "FFT has no energy at bin 20");

    // Round trip.
    std::mt19937 rng (1234);
    std::uniform_real_distribution<float> dist (-1.0f, 1.0f);
    std::vector<float> orig (256);
    for (auto& v : orig) v = dist (rng);
    std::copy (orig.begin(), orig.end(), re.begin());
    std::fill (im.begin(), im.end(), 0.0f);
    fft.forward (re.data(), im.data());
    fft.inverse (re.data(), im.data());
    double maxErr = 0.0;
    for (int i = 0; i < 256; ++i) maxErr = std::max (maxErr, (double) std::fabs (re[(size_t) i] - orig[(size_t) i]));
    tst::checkNear (maxErr, 0.0, 1e-5, "FFT forward/inverse round trip");
}

void testConvolver()
{
    tst::section ("Convolver (zero latency, partitioned)");

    std::mt19937 rng (99);
    std::uniform_real_distribution<float> dist (-1.0f, 1.0f);

    std::vector<float> ir (1500);
    for (size_t i = 0; i < ir.size(); ++i)
        ir[i] = dist (rng) * std::exp (-(float) i / 300.0f);

    std::vector<float> input (4096);
    for (auto& v : input) v = dist (rng);

    const auto expected = directConvolve (input, ir);

    Convolver conv;
    conv.prepare (48000.0, 512, 128);
    conv.setImpulseResponse (ir.data(), (int) ir.size());

    std::vector<float> actual = input;
    // Deliberately irregular block sizes: an ASIO driver will not always hand
    // you a power of two.
    const int blocks[] = { 64, 128, 37, 256, 1, 480, 512, 96 };
    int pos = 0, bi = 0;
    while (pos < (int) actual.size())
    {
        const int n = std::min (blocks[bi++ % 8], (int) actual.size() - pos);
        conv.process (actual.data() + pos, n);
        pos += n;
    }

    double maxErr = 0.0;
    for (size_t i = 0; i < actual.size(); ++i)
        maxErr = std::max (maxErr, (double) std::fabs (actual[i] - expected[i]));
    tst::checkNear (maxErr, 0.0, 2e-3, "partitioned convolution matches direct convolution");

    // Latency check: an impulse in must produce ir[0] on the very first sample.
    conv.reset();
    std::vector<float> impulse (512, 0.0f);
    impulse[0] = 1.0f;
    conv.process (impulse.data(), (int) impulse.size());
    tst::checkNear (impulse[0], ir[0], 1e-5, "first output sample equals ir[0] (zero latency)");
    tst::checkNear (impulse[200], ir[200], 1e-4, "tap 200 lands on sample 200");
    tst::checkNear (impulse[400], ir[400], 1e-4, "tap 400 lands on sample 400");
}

void testOversampler()
{
    tst::section ("Oversampler");

    for (int factor : { 2, 4 })
    {
        Oversampler os;
        os.prepare (48000.0, 512, factor);

        // A 100 Hz sine must survive the up/down round trip with its amplitude
        // and shape intact - this is the signal band the bass actually lives in.
        const int n = 4096;
        std::vector<float> in (n), out (n);
        for (int i = 0; i < n; ++i) in[(size_t) i] = 0.5f * std::sin (kTwoPi * 100.0f * i / 48000.0f);

        float* up = os.upsample (in.data(), n);
        (void) up;
        os.downsample (out.data(), n);

        float peak = 0.0f;
        for (int i = n / 2; i < n; ++i) peak = std::max (peak, std::fabs (out[(size_t) i]));
        tst::checkNear (peak, 0.5, 0.02, "oversample round trip preserves 100 Hz amplitude (x" + std::to_string (factor) + ")");

        bool finite = true;
        for (auto v : out) if (! std::isfinite (v)) finite = false;
        tst::check (finite, "oversampler output is finite (x" + std::to_string (factor) + ")");
    }
}

void testIrDesigner()
{
    tst::section ("IR designer");

    std::vector<FilterSpec> specs {
        { FilterSpec::Type::HighPass, 60.0f, 0.9f, 0.0f, 2 },
        { FilterSpec::Type::Peak,     90.0f, 1.2f, 4.0f, 1 },
        { FilterSpec::Type::LowPass,  3500.0f, 0.8f, 0.0f, 2 }
    };

    std::vector<float> ir;
    IrDesigner::designMinimumPhase (specs, 48000.0, 2048, ir);
    IrDesigner::normaliseAt (ir, 48000.0, 110.0f);

    tst::check (ir.size() == 2048, "IR has the requested length");

    bool finite = true;
    for (auto v : ir) if (! std::isfinite (v)) finite = false;
    tst::check (finite, "IR is finite");

    // Minimum phase: the bulk of the energy must be at the front.
    double total = 0.0, firstEighth = 0.0;
    for (size_t i = 0; i < ir.size(); ++i)
    {
        total += ir[i] * ir[i];
        if (i < ir.size() / 8) firstEighth += ir[i] * ir[i];
    }
    tst::check (firstEighth / total > 0.9, "IR energy is front-loaded (minimum phase)");

    // The realised response must track the designed magnitude.
    auto responseAt = [&ir] (float f)
    {
        double sre = 0.0, sim = 0.0;
        const double w = 2.0 * kPiD * f / 48000.0;
        for (size_t i = 0; i < ir.size(); ++i) { sre += ir[i] * std::cos (w * (double) i); sim -= ir[i] * std::sin (w * (double) i); }
        return (float) std::sqrt (sre * sre + sim * sim);
    };

    const float ref = responseAt (110.0f);
    tst::checkNear (gainToDb (ref), 0.0, 0.5, "IR normalised to 0 dB at 110 Hz");

    const float designed30  = IrDesigner::magnitudeOf (specs, 30.0f, 48000.0)
                            / IrDesigner::magnitudeOf (specs, 110.0f, 48000.0);
    tst::checkNear (gainToDb (responseAt (30.0f)), gainToDb (designed30), 1.5, "IR matches design at 30 Hz");

    const float designed1k = IrDesigner::magnitudeOf (specs, 1000.0f, 48000.0)
                           / IrDesigner::magnitudeOf (specs, 110.0f, 48000.0);
    tst::checkNear (gainToDb (responseAt (1000.0f)), gainToDb (designed1k), 1.0, "IR matches design at 1 kHz");

    const float designed6k = IrDesigner::magnitudeOf (specs, 6000.0f, 48000.0)
                           / IrDesigner::magnitudeOf (specs, 110.0f, 48000.0);
    tst::checkNear (gainToDb (responseAt (6000.0f)), gainToDb (designed6k), 2.0, "IR matches design at 6 kHz");
}
