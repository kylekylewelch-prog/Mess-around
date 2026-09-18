#include "bassamp/Cabinet.h"

namespace bassamp {

const char* cabName (CabType c)
{
    switch (c)
    {
        case CabType::Bypass: return "No Cab (DI)";
        case CabType::C1x12:  return "1x12";
        case CabType::C1x15:  return "1x15";
        case CabType::C2x10:  return "2x10";
        case CabType::C2x12:  return "2x12";
        case CabType::C4x10:  return "4x10";
        case CabType::C4x12:  return "4x12";
        case CabType::C8x10:  return "8x10";
        case CabType::UserIr: return "User IR";
        default:              return "?";
    }
}

const char* cabDescription (CabType c)
{
    switch (c)
    {
        case CabType::Bypass: return "Straight out of the amp, no speaker. Studio DI sound.";
        case CabType::C1x12:  return "Compact ported 1x12. Tight, mid-forward, quick transient - modern combo voicing.";
        case CabType::C1x15:  return "Sealed 1x15. Round, warm, early top-end rolloff. The vintage flip-top sound.";
        case CabType::C2x10:  return "Ported 2x10. Fast and articulate, strong upper mids, light below 60 Hz.";
        case CabType::C2x12:  return "Ported 2x12. Splits the difference: 15-style warmth with 10-style attack.";
        case CabType::C4x10:  return "Ported 4x10 with horn. The modern workhorse - extended lows, present top.";
        case CabType::C4x12:  return "Sealed 4x12. Thick low mids, aggressive push, rolls off early up top.";
        case CabType::C8x10:  return "Sealed 8x10 fridge. Huge midrange authority, controlled lows, no horn.";
        case CabType::UserIr: return "User-loaded impulse response.";
        default:              return "";
    }
}

const char* micName (MicType m)
{
    switch (m)
    {
        case MicType::DynamicLarge: return "Dynamic (large)";
        case MicType::DynamicSmall: return "Dynamic (small)";
        case MicType::Condenser:    return "Condenser";
        case MicType::Ribbon:       return "Ribbon";
        default:                    return "?";
    }
}

bool cabHasHorn (CabType c)
{
    return c == CabType::C4x10 || c == CabType::C2x10 || c == CabType::C1x12;
}

// The voicing of a bass cabinet is dominated by four things: the low-frequency
// alignment (sealed cabs roll off at 12 dB/oct from their system resonance,
// ported cabs at 24 dB/oct from tuning), the mass-controlled rolloff of the
// drivers up top, cone breakup between roughly 700 Hz and 3 kHz, and the
// baffle/box geometry. Each cab below is described in those terms.
std::vector<FilterSpec> buildCabinetVoicing (const CabinetSettings& s)
{
    using T = FilterSpec::Type;
    std::vector<FilterSpec> v;

    auto add = [&v] (T type, float f, float q, float g = 0.0f, int repeats = 1)
    {
        v.push_back ({ type, f, q, g, repeats });
    };

    switch (s.cab)
    {
        case CabType::C1x12:
            add (T::HighPass, 62.0f, 0.85f, 0.0f, 2);      // ported, 24 dB/oct
            add (T::Peak,     92.0f, 1.10f, 3.0f);         // port/driver alignment bump
            add (T::Peak,    240.0f, 0.90f, -1.5f);
            add (T::Peak,    850.0f, 1.20f, 2.0f);         // the 12" mid push
            add (T::Peak,   1900.0f, 1.80f, 2.5f);         // cone breakup
            add (T::LowPass, 4200.0f, 0.75f, 0.0f, 2);
            break;

        case CabType::C1x15:
            add (T::HighPass, 48.0f, 1.00f, 0.0f);         // sealed: 12 dB/oct
            add (T::Peak,     72.0f, 1.30f, 4.0f);         // system resonance, well damped
            add (T::Peak,    180.0f, 0.80f, 1.5f);         // the warmth people buy a 15 for
            add (T::Peak,    900.0f, 1.10f, -3.0f);        // 15s go soft through here
            add (T::Peak,   1600.0f, 2.20f, 2.0f);
            add (T::LowPass, 3000.0f, 0.70f, 0.0f, 2);     // dark, early rolloff
            break;

        case CabType::C2x10:
            add (T::HighPass, 66.0f, 0.90f, 0.0f, 2);
            add (T::Peak,     98.0f, 1.20f, 2.5f);
            add (T::Peak,    300.0f, 0.80f, -2.0f);
            add (T::Peak,    800.0f, 1.00f, 2.5f);         // articulation
            add (T::Peak,   2600.0f, 1.60f, 3.0f);         // 10" breakup: the "bite"
            add (T::LowPass, 5500.0f, 0.80f, 0.0f, 2);
            break;

        case CabType::C2x12:
            add (T::HighPass, 55.0f, 0.90f, 0.0f, 2);
            add (T::Peak,     80.0f, 1.10f, 3.0f);
            add (T::Peak,    500.0f, 0.90f, -2.0f);
            add (T::Peak,   1300.0f, 1.50f, 2.0f);
            add (T::LowPass, 4000.0f, 0.75f, 0.0f, 2);
            break;

        case CabType::C4x10:
            add (T::HighPass, 45.0f, 1.10f, 0.0f, 2);      // ported low, tuned deep
            add (T::Peak,     62.0f, 1.30f, 4.0f);
            add (T::Peak,    150.0f, 0.90f, -1.5f);
            add (T::Peak,    800.0f, 1.00f, 2.0f);
            add (T::Peak,   2500.0f, 1.50f, 3.5f);
            add (T::LowPass, 5000.0f, 0.80f, 0.0f, 2);
            break;

        case CabType::C4x12:
            add (T::HighPass, 58.0f, 1.00f, 0.0f);         // sealed
            add (T::Peak,    120.0f, 0.80f, 4.0f);         // the low-mid wall
            add (T::Peak,    700.0f, 1.00f, -3.0f);
            add (T::Peak,   1800.0f, 1.80f, 2.0f);
            add (T::LowPass, 3600.0f, 0.75f, 0.0f, 2);
            break;

        case CabType::C8x10:
            add (T::HighPass, 58.0f, 0.70f, 0.0f);         // sealed, gently damped
            add (T::Peak,    105.0f, 0.70f, 3.0f);         // broad push, not a bump
            add (T::Peak,    260.0f, 0.90f, -1.0f);
            add (T::Peak,    700.0f, 0.90f, 2.5f);         // where the fridge lives in a mix
            add (T::Peak,   1500.0f, 1.60f, 2.5f);
            add (T::Peak,   2600.0f, 2.00f, 1.5f);
            add (T::LowPass, 3200.0f, 0.80f, 0.0f, 2);     // no horn: hard rolloff
            break;

        default:
            break;
    }

    if (s.hornEnabled && cabHasHorn (s.cab))
    {
        // A horn does not extend the cab's rolloff, it adds a separate band on
        // top of it - hence a shelf plus its own bandpass shape.
        add (T::HighShelf, 4500.0f, 0.70f, 8.0f);
        add (T::Peak,      7000.0f, 1.20f, 2.0f);
        add (T::LowPass,  14000.0f, 0.70f, 0.0f);
    }

    // --- microphone ---------------------------------------------------------
    switch (s.mic)
    {
        case MicType::DynamicLarge:
            add (T::HighPass,   28.0f, 0.70f, 0.0f);
            add (T::Peak,       80.0f, 0.80f, 4.0f);
            add (T::Peak,      350.0f, 0.90f, -3.5f);
            add (T::Peak,     3500.0f, 1.00f, 4.5f);
            add (T::LowPass,  9000.0f, 0.70f, 0.0f);
            break;
        case MicType::DynamicSmall:
            add (T::HighPass,   55.0f, 0.80f, 0.0f);
            add (T::Peak,      120.0f, 0.90f, -2.0f);
            add (T::Peak,      450.0f, 0.90f, 1.0f);
            add (T::Peak,     5200.0f, 1.10f, 5.0f);
            add (T::LowPass, 11000.0f, 0.70f, 0.0f);
            break;
        case MicType::Condenser:
            add (T::HighPass,   22.0f, 0.70f, 0.0f);
            add (T::Peak,      200.0f, 0.80f, 0.5f);
            add (T::HighShelf, 8000.0f, 0.70f, 3.0f);
            add (T::LowPass, 18000.0f, 0.70f, 0.0f);
            break;
        case MicType::Ribbon:
            add (T::HighPass,   32.0f, 0.70f, 0.0f);
            add (T::Peak,      130.0f, 0.80f, 2.5f);
            add (T::HighShelf, 5000.0f, 0.70f, -7.0f);
            add (T::LowPass, 12000.0f, 0.70f, 0.0f);
            break;
        default: break;
    }

    // Mic position across the cone: on axis at the dust cap you get the driver's
    // full top end, out at the surround you lose it progressively.
    const float edge = clampf (s.micPosition, 0.0f, 1.0f);
    if (edge > 0.01f)
    {
        add (T::HighShelf, 2200.0f, 0.70f, -9.0f * edge);
        add (T::Peak,       420.0f, 1.00f,  1.5f * edge);
    }

    // Proximity effect: close in, a directional mic lifts the bottom several dB.
    const float dist = clampf (s.micDistance, 0.0f, 1.0f);
    const float proximity = (1.0f - dist) * (s.mic == MicType::Condenser ? 3.0f : 6.0f);
    if (proximity > 0.1f) add (T::LowShelf, 140.0f, 0.70f, proximity);
    if (dist > 0.01f)     add (T::HighShelf, 6000.0f, 0.70f, -3.0f * dist);   // air absorption

    // User low/high cut, applied last.
    if (s.lowCutHz  > 20.0f)    add (T::HighPass, s.lowCutHz,  0.707f, 0.0f, 2);
    if (s.highCutHz < 19000.0f) add (T::LowPass,  s.highCutHz, 0.707f, 0.0f, 2);

    return v;
}

void Cabinet::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate;
    convA.prepare (sampleRate, maxBlockSize, 128);
    convB.prepare (sampleRate, maxBlockSize, 128);
    scratch.assign ((size_t) std::max (1, maxBlockSize), 0.0f);
    crossfadeLength = std::max (32, (int) (sampleRate * 0.02));   // 20 ms equal-power swap
    crossfadeCounter = 0;
    activeIsA = true;
    swapRequested.store (false);

    current = CabinetSettings();
    designInto (convA, current);
    convB.setImpulseResponse (nullptr, 0);
    reset();
}

void Cabinet::reset()
{
    convA.reset();
    convB.reset();
    lowCut.reset();
    highCut.reset();
}

// Adds the parts of a cabinet's behaviour that are not magnitude response: the
// baffle/floor reflection that gives a miked cab its comb structure, and the
// room. Both are relative delays - nothing here shifts the IR as a whole, so the
// cabinet still adds zero latency.
void Cabinet::applyTimeDomainCharacter (std::vector<float>& ir, const CabinetSettings& s) const
{
    if (s.cab == CabType::Bypass || ir.empty()) return;

    const float dist = clampf (s.micDistance, 0.0f, 1.0f);

    // Path-length difference between the direct sound and the first boundary
    // reflection, in samples. Speed of sound 343 m/s.
    const float reflectionMetres = 0.18f + 0.55f * dist;
    const int   reflectionDelay  = (int) (reflectionMetres / 343.0f * (float) fs);
    const float reflectionGain   = -0.28f * (0.35f + 0.65f * dist);   // polarity inverted at the boundary

    std::vector<float> work = ir;
    if (reflectionDelay > 0 && reflectionDelay < (int) ir.size())
        for (size_t i = (size_t) reflectionDelay; i < ir.size(); ++i)
            work[i] += reflectionGain * ir[i - (size_t) reflectionDelay];

    // Room: a short diffuse tail, cheap to build and enough to stop the cab
    // sounding like it is in an anechoic chamber.
    const float room = clampf (s.roomAmount, 0.0f, 1.0f);
    if (room > 0.001f)
    {
        static const float tapTimesMs[] = { 7.3f, 11.9f, 17.1f, 23.4f, 31.7f, 41.3f };
        static const float tapGains[]   = { 0.42f, -0.33f, 0.27f, -0.21f, 0.16f, -0.12f };
        for (int t = 0; t < 6; ++t)
        {
            const int d = (int) (tapTimesMs[t] * 0.001f * (float) fs);
            if (d <= 0 || d >= (int) ir.size()) continue;
            const float g = tapGains[t] * room * 0.6f;
            for (size_t i = (size_t) d; i < ir.size(); ++i)
                work[i] += g * ir[i - (size_t) d];
        }
    }

    ir.swap (work);
}

void Cabinet::designInto (Convolver& target, const CabinetSettings& s)
{
    if (s.cab == CabType::Bypass)
    {
        currentVoicing.clear();
        static const float unitImpulse = 1.0f;
        target.setImpulseResponse (&unitImpulse, 1);
        return;
    }

    if (s.cab == CabType::UserIr && userIrLoaded && ! userIr.empty())
    {
        currentVoicing.clear();
        std::vector<float> ir = userIr;
        if (std::fabs (userIrRate - fs) > 1.0) ir = resampleIr (ir, userIrRate, fs);
        if ((int) ir.size() > kMaxUserIr) ir.resize ((size_t) kMaxUserIr);
        // Short fade at the end so truncation does not leave a step.
        const int fade = std::min (256, (int) ir.size() / 8);
        for (int i = 0; i < fade; ++i)
        {
            const float w = 0.5f * (1.0f + std::cos (kPi * (float) i / (float) fade));
            ir[ir.size() - (size_t) fade + (size_t) i] *= w;
        }
        target.setImpulseResponse (ir.data(), (int) ir.size());
        return;
    }

    currentVoicing = buildCabinetVoicing (s);
    std::vector<float> ir;
    IrDesigner::designMinimumPhase (currentVoicing, fs, kIrLength, ir);
    applyTimeDomainCharacter (ir, s);
    IrDesigner::normaliseAt (ir, fs, 110.0f);
    target.setImpulseResponse (ir.data(), (int) ir.size());
}

bool Cabinet::applySettings (const CabinetSettings& s)
{
    if (swapRequested.load (std::memory_order_acquire))
        return false;   // audio thread has not consumed the previous swap yet

    current = s;
    designInto (activeIsA ? convB : convA, s);   // build into the inactive slot
    swapRequested.store (true, std::memory_order_release);
    return true;
}

std::string Cabinet::loadUserIr (const std::string& path)
{
    WavData wav = readWavFile (path);
    if (! wav.valid) return wav.error.empty() ? "Could not read IR file" : wav.error;
    if (wav.samples.empty()) return "IR file contains no samples";

    userIr = std::move (wav.samples);
    userIrRate = wav.sampleRate;
    userIrLoaded = true;

    const size_t slash = path.find_last_of ("/\\");
    userIrName = (slash == std::string::npos) ? path : path.substr (slash + 1);
    return {};
}

void Cabinet::process (float* data, int numSamples)
{
    if (numSamples <= 0) return;
    if ((int) scratch.size() < numSamples) scratch.assign ((size_t) numSamples, 0.0f);

    if (swapRequested.load (std::memory_order_acquire) && crossfadeCounter == 0)
        crossfadeCounter = crossfadeLength;

    Convolver& active   = activeIsA ? convA : convB;
    Convolver& incoming = activeIsA ? convB : convA;

    if (crossfadeCounter > 0)
    {
        std::copy (data, data + numSamples, scratch.begin());
        active.process (data, numSamples);
        incoming.process (scratch.data(), numSamples);

        for (int n = 0; n < numSamples; ++n)
        {
            if (crossfadeCounter > 0)
            {
                const float t = 1.0f - (float) crossfadeCounter / (float) crossfadeLength;
                // Equal power: the two IRs are not correlated, so a linear fade
                // would dip in the middle.
                const float gIn  = std::sin (0.5f * kPi * t);
                const float gOut = std::cos (0.5f * kPi * t);
                data[n] = data[n] * gOut + scratch[(size_t) n] * gIn;
                --crossfadeCounter;
            }
            else
            {
                data[n] = scratch[(size_t) n];
            }
        }

        if (crossfadeCounter == 0)
        {
            activeIsA = ! activeIsA;
            swapRequested.store (false, std::memory_order_release);
        }
    }
    else
    {
        active.process (data, numSamples);
    }
}

float Cabinet::magnitudeDbAt (float freq) const
{
    if (currentVoicing.empty()) return 0.0f;
    const float m   = IrDesigner::magnitudeOf (currentVoicing, freq, fs);
    const float ref = IrDesigner::magnitudeOf (currentVoicing, 110.0f, fs);
    return gainToDb (m / std::max (ref, 1.0e-9f));
}

} // namespace bassamp
