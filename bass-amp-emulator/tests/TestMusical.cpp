// Tuner, metronome, amp and cabinet behaviour.
#include "TestHarness.h"
#include "bassamp/Tuner.h"
#include "bassamp/Metronome.h"
#include "bassamp/Amp.h"
#include "bassamp/Cabinet.h"
#include "bassamp/Pedalboard.h"
#include <vector>

using namespace bassamp;

// A bass note is not a sine wave - it is a fundamental plus a long harmonic
// series, and the harmonics are usually louder than the fundamental. Detection
// has to survive that, so the test signal reflects it.
static std::vector<float> bassTone (float freq, double fs, float seconds)
{
    const int n = (int) (fs * seconds);
    std::vector<float> out ((size_t) n, 0.0f);
    static const float harmonicGains[] = { 0.55f, 1.0f, 0.75f, 0.45f, 0.30f, 0.18f, 0.10f, 0.06f };
    for (int i = 0; i < n; ++i)
    {
        float v = 0.0f;
        for (int h = 0; h < 8; ++h)
            v += harmonicGains[h] * std::sin (kTwoPi * freq * (float) (h + 1) * (float) i / (float) fs
                                              + (float) h * 0.7f);
        // Pluck envelope.
        const float env = std::exp (-(float) i / (float) (fs * 1.8));
        out[(size_t) i] = 0.22f * v * (0.35f + 0.65f * env);
    }
    return out;
}

void testTuner()
{
    tst::section ("Strobe tuner");

    struct Case { float freq; int midi; const char* label; };
    const Case cases[] =
    {
        { 30.868f, 23, "B0 (low B, 5-string)" },
        { 41.203f, 28, "E1 (open E)" },
        { 55.000f, 33, "A1" },
        { 73.416f, 38, "D2" },
        { 97.999f, 43, "G2" },
        { 146.83f, 50, "D3 (12th fret D string)" }
    };

    for (const auto& c : cases)
    {
        Tuner tuner;
        tuner.prepare (48000.0, 256);
        tuner.setAutoTarget (true);

        auto tone = bassTone (c.freq, 48000.0, 1.2f);
        for (size_t pos = 0; pos < tone.size(); pos += 256)
        {
            const int n = (int) std::min ((size_t) 256, tone.size() - pos);
            tuner.processAudio (tone.data() + pos, n);
            tuner.runDetection();
        }

        tst::check (tuner.hasSignal(), std::string ("tuner locks onto ") + c.label);
        tst::check (tuner.getMidiNote() == c.midi,
                    std::string ("tuner names ") + c.label + " correctly (got MIDI "
                    + std::to_string (tuner.getMidiNote()) + ")");
        tst::checkNear (tuner.getCents(), 0.0, 3.0, std::string ("tuner cents accurate on ") + c.label);
    }

    // Detuned string: 20 cents sharp of A1 must read as A1, +20 cents.
    {
        Tuner tuner;
        tuner.prepare (48000.0, 256);
        const float sharp = 55.0f * std::pow (2.0f, 20.0f / 1200.0f);
        auto tone = bassTone (sharp, 48000.0, 1.2f);
        for (size_t pos = 0; pos < tone.size(); pos += 256)
        {
            const int n = (int) std::min ((size_t) 256, tone.size() - pos);
            tuner.processAudio (tone.data() + pos, n);
            tuner.runDetection();
        }
        tst::check (tuner.getMidiNote() == 33, "20 cents sharp still reads as A1");
        tst::checkNear (tuner.getCents(), 20.0, 4.0, "tuner reports +20 cents");
    }

    // Silence must not produce a reading.
    {
        Tuner tuner;
        tuner.prepare (48000.0, 256);
        std::vector<float> silence (48000, 0.0f);
        for (size_t pos = 0; pos < silence.size(); pos += 256)
        {
            tuner.processAudio (silence.data() + pos, 256);
            tuner.runDetection();
        }
        tst::check (! tuner.hasSignal(), "tuner reports no signal on silence");
    }
}

static std::vector<int> findClickOnsets (const std::vector<float>& audio, float threshold = 0.05f)
{
    std::vector<int> onsets;
    bool armed = true;
    int quiet = 0;
    for (size_t i = 0; i < audio.size(); ++i)
    {
        if (std::fabs (audio[i]) > threshold)
        {
            if (armed) { onsets.push_back ((int) i); armed = false; }
            quiet = 0;
        }
        else if (++quiet > 200) armed = true;
    }
    return onsets;
}

void testMetronome()
{
    tst::section ("Metronome");

    const double fs = 48000.0;

    // 120 BPM, 4/4, quarters: clicks exactly 24000 samples apart.
    {
        Metronome m;
        m.prepare (fs, 128);
        m.setTempo (120.0);
        m.setTimeSignature (4, 4);
        m.setSubdivision (Subdivision::Quarter);
        m.setLevel (1.0f);
        m.setRunning (true);

        std::vector<float> audio ((size_t) (fs * 4.0), 0.0f);
        for (size_t pos = 0; pos < audio.size(); pos += 128)
            m.process (audio.data() + pos, (int) std::min ((size_t) 128, audio.size() - pos));

        const auto onsets = findClickOnsets (audio);
        tst::check (onsets.size() == 8, "120 BPM over 4 s produces 8 quarter-note clicks (got "
                                        + std::to_string (onsets.size()) + ")");
        tst::check (! onsets.empty() && onsets[0] < 8, "first click lands on the downbeat, not a beat late");

        int maxDrift = 0;
        for (size_t i = 1; i < onsets.size(); ++i)
            maxDrift = std::max (maxDrift, std::abs ((onsets[i] - onsets[0]) - (int) i * 24000));
        tst::check (maxDrift <= 1, "no scheduler drift across 4 s (max " + std::to_string (maxDrift) + " samples)");
    }

    // The buffer size must not change the timing.
    {
        Metronome m;
        m.prepare (fs, 512);
        m.setTempo (137.0);
        m.setLevel (1.0f);
        m.setRunning (true);

        std::vector<float> audio ((size_t) (fs * 4.0), 0.0f);
        const int blocks[] = { 64, 37, 512, 1, 480, 128 };
        size_t pos = 0; int bi = 0;
        while (pos < audio.size())
        {
            const int n = (int) std::min ((size_t) blocks[bi++ % 6], audio.size() - pos);
            m.process (audio.data() + pos, n);
            pos += (size_t) n;
        }

        const auto onsets = findClickOnsets (audio);
        const double expected = 60.0 / 137.0 * fs;
        bool ok = onsets.size() > 4;
        for (size_t i = 1; ok && i < onsets.size(); ++i)
            ok = std::fabs ((onsets[i] - onsets[0]) - (double) i * expected) <= 1.5;
        tst::check (ok, "timing is identical under irregular block sizes");
    }

    // 6/8 is felt in two, not six.
    {
        Metronome m;
        m.prepare (fs, 128);
        m.setTimeSignature (6, 8);
        tst::check (m.getBeatsPerBar() == 2, "6/8 counts two dotted-quarter pulses per bar");
        m.setTimeSignature (7, 8);
        tst::check (m.getBeatsPerBar() == 7, "7/8 counts seven eighth-note pulses per bar");
        m.setTimeSignature (5, 4);
        tst::check (m.getBeatsPerBar() == 5, "5/4 counts five");
    }

    // Tap tempo.
    {
        Metronome m;
        m.prepare (fs, 128);
        for (int i = 0; i < 5; ++i) m.tap (i * 0.5);   // 120 BPM
        tst::checkNear (m.getTempo(), 120.0, 0.5, "tap tempo resolves to 120 BPM");
    }
}

void testAmpAndCab()
{
    tst::section ("Amps and cabinets");

    for (int t = 0; t < (int) AmpType::NumTypes; ++t)
    {
        Amp amp;
        amp.prepare (48000.0, 256, 2);

        AmpSettings s;
        s.type = (AmpType) t;
        s.gain = 0.85f;      // driven hard on purpose
        s.master = 0.9f;
        s.bass = 0.7f; s.mid = 0.4f; s.treble = 0.6f;
        s.presence = 0.5f;
        amp.setSettings (s);

        auto tone = bassTone (41.2f, 48000.0, 0.6f);
        for (auto& v : tone) v *= 3.0f;   // slam the input

        for (size_t pos = 0; pos < tone.size(); pos += 256)
            amp.process (tone.data() + pos, (int) std::min ((size_t) 256, tone.size() - pos));

        bool finite = true, bounded = true;
        for (auto v : tone)
        {
            if (! std::isfinite (v)) finite = false;
            if (std::fabs (v) > 8.0f) bounded = false;
        }
        tst::check (finite, std::string (ampName ((AmpType) t)) + " stays finite when overdriven");
        tst::check (bounded, std::string (ampName ((AmpType) t)) + " output stays bounded");
    }

    // Every cabinet must roll off below its tuning and above the driver's limit,
    // which is the whole reason a DI track sounds wrong without one.
    for (int c = (int) CabType::C1x12; c <= (int) CabType::C8x10; ++c)
    {
        Cabinet cab;
        cab.prepare (48000.0, 256);
        CabinetSettings cs;
        cs.cab = (CabType) c;
        cs.roomAmount = 0.0f;
        while (! cab.applySettings (cs))
        {
            std::vector<float> flush (256, 0.0f);
            cab.process (flush.data(), 256);
        }

        // Let the crossfade complete.
        std::vector<float> flush (4096, 0.0f);
        cab.process (flush.data(), 4096);

        auto measure = [&cab] (float freq)
        {
            Cabinet& c2 = cab;
            const int n = 16384;
            std::vector<float> sig ((size_t) n);
            for (int i = 0; i < n; ++i) sig[(size_t) i] = std::sin (kTwoPi * freq * (float) i / 48000.0f);
            c2.process (sig.data(), n);
            float peak = 0.0f;
            for (int i = n / 2; i < n; ++i) peak = std::max (peak, std::fabs (sig[(size_t) i]));
            return peak;
        };

        const float ref = measure (110.0f);
        const float low = measure (25.0f);
        const float high = measure (9000.0f);

        tst::check (ref > 0.2f, std::string (cabName ((CabType) c)) + " passes 110 Hz");
        tst::check (gainToDb (low / ref) < -9.0f,
                    std::string (cabName ((CabType) c)) + " rolls off below its tuning");
        tst::check (gainToDb (high / ref) < -18.0f,
                    std::string (cabName ((CabType) c)) + " rolls off above the driver's range");
    }
}

void testPedals()
{
    tst::section ("Pedals");

    Pedalboard board;
    board.prepare (48000.0, 256);

    for (int i = 0; i < Pedalboard::kNumSlots; ++i)
    {
        Pedal* p = board.pedalOfType ((PedalType) i);
        tst::check (p != nullptr, std::string ("pedal ") + pedalTypeName ((PedalType) i) + " is constructed");
        if (p == nullptr) continue;

        tst::check (p->numParams() > 0 && p->numParams() <= Pedal::kMaxParams,
                    std::string (p->name()) + " has a sane parameter count");

        p->setEnabled (true);
        auto tone = bassTone (55.0f, 48000.0, 0.4f);

        // Push every parameter to both extremes: a pedal that only behaves at its
        // defaults is a pedal that will blow up on stage.
        for (int extreme = 0; extreme < 2; ++extreme)
        {
            for (int q = 0; q < p->numParams(); ++q)
                p->setParamValue (q, extreme == 0 ? p->param (q).minValue : p->param (q).maxValue);

            auto copy = tone;
            for (size_t pos = 0; pos < copy.size(); pos += 256)
                p->process (copy.data() + pos, (int) std::min ((size_t) 256, copy.size() - pos));

            bool finite = true, bounded = true;
            for (auto v : copy)
            {
                if (! std::isfinite (v)) finite = false;
                if (std::fabs (v) > 25.0f) bounded = false;
            }
            tst::check (finite, std::string (p->name()) + " stays finite at parameter extremes");
            tst::check (bounded, std::string (p->name()) + " stays bounded at parameter extremes");
        }

        p->restoreDefaults();
        p->setEnabled (false);
    }

    // Reordering must survive and must actually change the chain order.
    board.moveTo (PedalType::Fuzz, 0);
    tst::check (board.chainIndexOfType (PedalType::Fuzz) == 0, "pedal can be moved to the front of the chain");
    board.moveTo (PedalType::Fuzz, Pedalboard::kNumSlots - 1);
    tst::check (board.chainIndexOfType (PedalType::Fuzz) == Pedalboard::kNumSlots - 1,
                "pedal can be moved to the end of the chain");

    auto order = board.getOrder();
    bool seen[Pedalboard::kNumSlots] = { };
    bool complete = true;
    for (int i = 0; i < Pedalboard::kNumSlots; ++i)
    {
        if (order[(size_t) i] < 0 || order[(size_t) i] >= Pedalboard::kNumSlots || seen[order[(size_t) i]])
            complete = false;
        else seen[order[(size_t) i]] = true;
    }
    tst::check (complete, "chain order stays a valid permutation after reordering");
}
