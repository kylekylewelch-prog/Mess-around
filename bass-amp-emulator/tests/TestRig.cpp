// Whole-rig and preset behaviour.
#include "TestHarness.h"
#include "bassamp/Presets.h"
#include <vector>

using namespace bassamp;

static std::vector<float> pluckedNote (float freq, double fs, float seconds)
{
    const int n = (int) (fs * seconds);
    std::vector<float> out ((size_t) n, 0.0f);
    static const float gains[] = { 0.6f, 1.0f, 0.7f, 0.4f, 0.25f, 0.15f, 0.08f, 0.05f };
    for (int i = 0; i < n; ++i)
    {
        float v = 0.0f;
        for (int h = 0; h < 8; ++h)
            v += gains[h] * std::sin (kTwoPi * freq * (float) (h + 1) * (float) i / (float) fs + (float) h);
        const float env = std::exp (-(float) i / (float) (fs * 1.2));
        out[(size_t) i] = 0.3f * v * env;
    }
    return out;
}

void testRig()
{
    tst::section ("Rig");

    Rig rig;
    rig.prepare (48000.0, 256);

    auto input = pluckedNote (41.2f, 48000.0, 0.5f);
    std::vector<float> left (input.size(), 0.0f), right (input.size(), 0.0f);

    for (size_t pos = 0; pos < input.size(); pos += 256)
    {
        const int n = (int) std::min ((size_t) 256, input.size() - pos);
        rig.process (input.data() + pos, left.data() + pos, right.data() + pos, n);
    }

    bool finite = true, matched = true;
    for (size_t i = 0; i < left.size(); ++i)
    {
        if (! std::isfinite (left[i]) || ! std::isfinite (right[i])) finite = false;
        if (left[i] != right[i]) matched = false;
    }
    tst::check (finite, "rig output is finite");
    tst::check (matched, "rig output channels match (mono source)");
    tst::check (rig.getInputLevel() > 0.05f, "input meter registers signal");

    // Zero latency end to end: an impulse at the input must produce output in
    // the same block, not one buffer later.
    {
        Rig r2;
        r2.prepare (48000.0, 64);
        CabinetSettings cs;
        cs.cab = CabType::C8x10;
        r2.requestCabinetSettings (cs);
        r2.serviceMessageThread();

        std::vector<float> impulse (64, 0.0f), l (64, 0.0f), rr (64, 0.0f);
        impulse[0] = 1.0f;
        r2.process (impulse.data(), l.data(), rr.data(), 64);

        float earlyEnergy = 0.0f;
        for (int i = 0; i < 16; ++i) earlyEnergy += std::fabs (l[(size_t) i]);
        tst::check (earlyEnergy > 1.0e-4f, "rig responds within the first 16 samples (no added latency)");
    }
}

void testPresets()
{
    tst::section ("Presets");

    tst::check (numPresets() >= 30, "library ships at least 30 presets (has "
                                    + std::to_string (numPresets()) + ")");
    tst::check (presetCategories().size() >= 10, "presets cover at least 10 genres");

    for (int i = 0; i < numPresets(); ++i)
    {
        const Preset& preset = getPreset (i);

        tst::check (! preset.name.empty() && ! preset.category.empty()
                    && ! preset.reference.empty() && ! preset.notes.empty(),
                    "preset " + std::to_string (i) + " is fully documented");

        Rig rig;
        rig.prepare (48000.0, 256);
        applyPreset (rig, preset);

        auto input = pluckedNote (55.0f, 48000.0, 0.5f);
        std::vector<float> l (input.size(), 0.0f), r (input.size(), 0.0f);
        for (size_t pos = 0; pos < input.size(); pos += 256)
        {
            const int n = (int) std::min ((size_t) 256, input.size() - pos);
            rig.process (input.data() + pos, l.data() + pos, r.data() + pos, n);
        }

        bool finite = true;
        float peak = 0.0f;
        for (auto v : l) { if (! std::isfinite (v)) finite = false; peak = std::max (peak, std::fabs (v)); }

        tst::check (finite, "preset '" + preset.name + "' produces finite output");
        tst::check (peak > 0.005f, "preset '" + preset.name + "' produces audible output");
        // A preset that clips the output on a normal note is a preset nobody can use.
        tst::check (peak < 2.0f, "preset '" + preset.name + "' does not run away (peak "
                                 + std::to_string (peak) + ")");
    }
}

void testStateRoundTrip()
{
    tst::section ("Preset save / load");

    Rig source;
    source.prepare (48000.0, 256);
    applyPreset (source, getPreset (findPreset ("Lead Bass Fuzz")));

    // Change a few things away from the preset so the round trip has to carry
    // more than just the preset defaults.
    source.pedalboard().moveTo (PedalType::Wah, 0);
    source.pedalboard().setPlacement (PedalType::Delay, PedalPlacement::PostCab);
    source.pedalboard().pedalOfType (PedalType::Fuzz)->setParamValue (0, 63.0f);
    source.metronome().setTempo (152.0);
    source.metronome().setTimeSignature (7, 8);

    const std::string state = source.saveState();
    tst::check (state.find ("version=1") != std::string::npos, "saved state carries a version");

    Rig target;
    target.prepare (48000.0, 256);
    tst::check (target.loadState (state), "state loads successfully");

    tst::check (target.amp().getSettings().type == source.amp().getSettings().type, "amp type round trips");
    tst::checkNear (target.amp().getSettings().gain, source.amp().getSettings().gain, 1e-4, "amp gain round trips");
    tst::check (target.getCabinetSettings().cab == source.getCabinetSettings().cab, "cabinet round trips");
    tst::check (target.pedalboard().chainIndexOfType (PedalType::Wah) == 0, "chain order round trips");
    tst::check (target.pedalboard().placementOf (PedalType::Delay) == PedalPlacement::PostCab,
                "pedal placement round trips");
    tst::checkNear (target.pedalboard().pedalOfType (PedalType::Fuzz)->getParamValue (0), 63.0, 0.01,
                    "pedal parameters round trip");
    tst::check (target.pedalboard().pedalOfType (PedalType::Fuzz)->isEnabled(), "pedal bypass state round trips");
    tst::checkNear (target.metronome().getTempo(), 152.0, 0.01, "tempo round trips");
    tst::check (target.metronome().getTimeSignature().numerator == 7
                && target.metronome().getTimeSignature().denominator == 8, "time signature round trips");

    tst::check (! target.loadState ("garbage without a version line\n"), "malformed state is rejected");
}
