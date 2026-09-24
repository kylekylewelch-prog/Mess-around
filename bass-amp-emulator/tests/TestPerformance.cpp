// Throughput measurement. Reported rather than asserted: CI runners are shared
// and a timing assertion would be flaky, but the numbers are the whole point of
// the project, so they belong in the test output where a change that doubles the
// cost of the amp section is impossible to miss.
#include "TestHarness.h"
#include "bassamp/Presets.h"
#include <chrono>
#include <vector>

using namespace bassamp;

namespace {

double measureRealtimeFactor (Rig& rig, int blockSize, double seconds = 4.0)
{
    const double fs = 48000.0;
    const int totalSamples = (int) (fs * seconds);

    std::vector<float> input ((size_t) blockSize), l ((size_t) blockSize), r ((size_t) blockSize);
    for (int i = 0; i < blockSize; ++i)
        input[(size_t) i] = 0.25f * std::sin (kTwoPi * 55.0f * (float) i / (float) fs);

    // Warm up so the first block's cache misses do not dominate a short run.
    for (int i = 0; i < 8; ++i) rig.process (input.data(), l.data(), r.data(), blockSize);

    const auto start = std::chrono::steady_clock::now();
    for (int done = 0; done < totalSamples; done += blockSize)
        rig.process (input.data(), l.data(), r.data(), blockSize);
    const auto end = std::chrono::steady_clock::now();

    const double elapsed = std::chrono::duration<double> (end - start).count();
    return elapsed > 0.0 ? seconds / elapsed : 0.0;
}

} // namespace

void testPerformance()
{
    tst::section ("Throughput (informational)");

    struct Case { const char* preset; int oversampling; };
    const Case cases[] =
    {
        { "Flat Reference",   2 },
        { "Fridge Standard",  2 },
        { "Split Rig Metal",  2 },
        { "Split Rig Metal",  4 },
        { "Lead Bass Fuzz",   2 },
        { "Dub Delay",        2 }
    };

    for (const auto& c : cases)
    {
        const int index = findPreset (c.preset);
        if (index < 0) continue;

        Rig rig;
        RigSettings settings;
        settings.oversampling = c.oversampling;
        rig.setSettings (settings);
        rig.prepare (48000.0, 128);
        applyPreset (rig, getPreset (index));

        // applyPreset carries the rig's performance settings through, so set
        // oversampling again explicitly for this measurement.
        settings = rig.getSettings();
        settings.oversampling = c.oversampling;
        rig.setSettings (settings);

        const double factor = measureRealtimeFactor (rig, 128);
        std::printf ("  %-18s  %dx oversampling   %6.1fx realtime   (~%.1f%% of one core at 128 samples)\n",
                     c.preset, c.oversampling, factor, factor > 0.0 ? 100.0 / factor : 0.0);

        // Only a sanity floor, generous enough that a shared CI runner under
        // load will not trip it. The printed number is the useful part.
        tst::check (factor > 3.0, std::string ("preset '") + c.preset + "' runs comfortably faster than realtime");
    }
}
