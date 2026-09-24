#include "bassamp/Pedalboard.h"
#include <cstring>

namespace bassamp {

const char* pedalTypeName (PedalType type)
{
    switch (type)
    {
        case PedalType::Compressor:     return "Compressor";
        case PedalType::NoiseGate:      return "Noise Gate";
        case PedalType::Octave:         return "Octave";
        case PedalType::Synth:          return "Bass Synth";
        case PedalType::Fuzz:           return "Fuzz";
        case PedalType::Distortion:     return "Distortion";
        case PedalType::Overdrive:      return "Overdrive";
        case PedalType::Wah:            return "Wah";
        case PedalType::EnvelopeFilter: return "Envelope Filter";
        case PedalType::Chorus:         return "Chorus";
        case PedalType::Flanger:        return "Flanger";
        case PedalType::Phaser:         return "Phaser";
        case PedalType::Tremolo:        return "Tremolo";
        case PedalType::RingMod:        return "Ring Mod";
        case PedalType::BitCrusher:     return "Bit Crusher";
        case PedalType::GraphicEq:      return "Graphic EQ";
        case PedalType::Delay:          return "Delay";
        case PedalType::Reverb:         return "Reverb";
        default:                        return "?";
    }
}

const char* placementName (PedalPlacement p)
{
    switch (p)
    {
        case PedalPlacement::FrontOfAmp: return "Front of amp";
        case PedalPlacement::AmpLoop:    return "Amp FX loop";
        case PedalPlacement::PostCab:    return "After cab";
        default:                         return "?";
    }
}

PedalPtr createPedal (PedalType type)
{
    switch (type)
    {
        case PedalType::Compressor:     return std::make_unique<CompressorPedal>();
        case PedalType::NoiseGate:      return std::make_unique<NoiseGatePedal>();
        case PedalType::Octave:         return std::make_unique<OctavePedal>();
        case PedalType::Synth:          return std::make_unique<SynthPedal>();
        case PedalType::Fuzz:           return std::make_unique<FuzzPedal>();
        case PedalType::Distortion:     return std::make_unique<DistortionPedal>();
        case PedalType::Overdrive:      return std::make_unique<OverdrivePedal>();
        case PedalType::Wah:            return std::make_unique<WahPedal>();
        case PedalType::EnvelopeFilter: return std::make_unique<EnvelopeFilterPedal>();
        case PedalType::Chorus:         return std::make_unique<ChorusPedal>();
        case PedalType::Flanger:        return std::make_unique<FlangerPedal>();
        case PedalType::Phaser:         return std::make_unique<PhaserPedal>();
        case PedalType::Tremolo:        return std::make_unique<TremoloPedal>();
        case PedalType::RingMod:        return std::make_unique<RingModPedal>();
        case PedalType::BitCrusher:     return std::make_unique<BitCrusherPedal>();
        case PedalType::GraphicEq:      return std::make_unique<GraphicEqPedal>();
        case PedalType::Delay:          return std::make_unique<DelayPedal>();
        case PedalType::Reverb:         return std::make_unique<ReverbPedal>();
        default:                        return nullptr;
    }
}

// The default order is the one most players actually use: gate and compressor
// first so everything downstream sees a controlled signal, pitch and dirt next,
// filters after the dirt, modulation, then time effects last.
static const PedalType kDefaultOrder[] =
{
    PedalType::NoiseGate,
    PedalType::Compressor,
    PedalType::Octave,
    PedalType::Synth,
    PedalType::Fuzz,
    PedalType::Distortion,
    PedalType::Overdrive,
    PedalType::EnvelopeFilter,
    PedalType::Wah,
    PedalType::GraphicEq,
    PedalType::BitCrusher,
    PedalType::RingMod,
    PedalType::Phaser,
    PedalType::Flanger,
    PedalType::Chorus,
    PedalType::Tremolo,
    PedalType::Delay,
    PedalType::Reverb
};

static PedalPlacement defaultPlacementFor (PedalType t)
{
    switch (t)
    {
        case PedalType::Delay:
        case PedalType::Reverb:  return PedalPlacement::AmpLoop;
        default:                 return PedalPlacement::FrontOfAmp;
    }
}

Pedalboard::Pedalboard()
{
    for (int i = 0; i < kNumSlots; ++i)
        pedals[i] = createPedal ((PedalType) i);

    restoreDefaultOrder();
}

void Pedalboard::restoreDefaultOrder()
{
    static_assert (sizeof (kDefaultOrder) / sizeof (PedalType) == (size_t) kNumSlots,
                   "default pedal order must list every pedal exactly once");

    for (int i = 0; i < kNumSlots; ++i)
    {
        editing.order[i] = (int) kDefaultOrder[i];
        editing.placement[(int) kDefaultOrder[i]] = (uint8_t) defaultPlacementFor (kDefaultOrder[i]);
    }
    publish (editing);
}

void Pedalboard::prepare (double sampleRate, int maxBlockSize)
{
    for (auto& p : pedals) if (p) p->prepare (sampleRate, maxBlockSize);
    audioCopy = editing;
}

void Pedalboard::reset()
{
    for (auto& p : pedals) if (p) p->reset();
}

void Pedalboard::setTempo (double bpm)
{
    for (auto& p : pedals) if (p) p->setTempo (bpm);
}

// Seqlock publish: odd sequence means "write in progress", so the audio thread
// knows to keep using the copy it already has.
void Pedalboard::publish (const ChainConfig& cfg)
{
    sequence.fetch_add (1, std::memory_order_acq_rel);
    std::atomic_thread_fence (std::memory_order_release);
    shared = cfg;
    std::atomic_thread_fence (std::memory_order_release);
    sequence.fetch_add (1, std::memory_order_acq_rel);
}

Pedalboard::ChainConfig Pedalboard::readConfig() const
{
    const uint32_t before = sequence.load (std::memory_order_acquire);
    if ((before & 1u) == 0u)
    {
        ChainConfig candidate = shared;
        std::atomic_thread_fence (std::memory_order_acquire);
        if (sequence.load (std::memory_order_acquire) == before)
        {
            audioCopy = candidate;
            return candidate;
        }
    }
    return audioCopy;   // a write was in flight; use the last good layout
}

void Pedalboard::process (float* data, int numSamples, PedalPlacement where)
{
    const ChainConfig cfg = readConfig();

    for (int i = 0; i < kNumSlots; ++i)
    {
        const int idx = cfg.order[i];
        if (idx < 0 || idx >= kNumSlots) continue;
        if ((PedalPlacement) cfg.placement[idx] != where) continue;

        Pedal* p = pedals[idx].get();
        if (p != nullptr && p->isEnabled()) p->process (data, numSamples);
    }
}

Pedal* Pedalboard::pedalAt (int chainIndex)
{
    if (chainIndex < 0 || chainIndex >= kNumSlots) return nullptr;
    const int idx = editing.order[chainIndex];
    return (idx >= 0 && idx < kNumSlots) ? pedals[idx].get() : nullptr;
}

Pedal* Pedalboard::pedalOfType (PedalType type)
{
    const int i = (int) type;
    return (i >= 0 && i < kNumSlots) ? pedals[i].get() : nullptr;
}

int Pedalboard::chainIndexOfType (PedalType type) const
{
    for (int i = 0; i < kNumSlots; ++i) if (editing.order[i] == (int) type) return i;
    return -1;
}

PedalPlacement Pedalboard::placementOf (PedalType type) const
{
    return (PedalPlacement) editing.placement[(int) type];
}

void Pedalboard::setPlacement (PedalType type, PedalPlacement placement)
{
    editing.placement[(int) type] = (uint8_t) placement;
    publish (editing);
}

void Pedalboard::moveTo (PedalType type, int newChainIndex)
{
    const int from = chainIndexOfType (type);
    const int to = std::max (0, std::min (newChainIndex, kNumSlots - 1));
    if (from < 0 || from == to) return;

    int tmp[kNumSlots];
    int w = 0;
    for (int i = 0; i < kNumSlots; ++i) if (i != from) tmp[w++] = editing.order[i];

    int out[kNumSlots];
    int r = 0;
    for (int i = 0; i < kNumSlots; ++i)
        out[i] = (i == to) ? (int) type : tmp[r++];

    std::memcpy (editing.order, out, sizeof (out));
    publish (editing);
}

std::array<int, Pedalboard::kNumSlots> Pedalboard::getOrder() const
{
    std::array<int, kNumSlots> out { };
    for (int i = 0; i < kNumSlots; ++i) out[(size_t) i] = editing.order[i];
    return out;
}

void Pedalboard::setOrder (const std::array<int, kNumSlots>& order)
{
    bool seen[kNumSlots] = { };
    int w = 0;
    int tmp[kNumSlots];
    for (int i = 0; i < kNumSlots; ++i)
    {
        const int v = order[(size_t) i];
        if (v >= 0 && v < kNumSlots && ! seen[v]) { seen[v] = true; tmp[w++] = v; }
    }
    // Anything the caller left out keeps its place at the back, so a preset saved
    // by an older build still loads cleanly.
    for (int i = 0; i < kNumSlots; ++i) if (! seen[i]) tmp[w++] = i;

    std::memcpy (editing.order, tmp, sizeof (tmp));
    publish (editing);
}

} // namespace bassamp
