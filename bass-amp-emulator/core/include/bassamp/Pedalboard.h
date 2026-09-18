// Pedalboard.h - the ordered effects chain.
//
// Order matters on a real board (fuzz into wah sounds nothing like wah into
// fuzz), so the chain is reorderable, and each pedal can sit in front of the
// amp, in the amp's effects loop, or after the cabinet. Reordering happens on
// the message thread while audio is running, so the layout is published to the
// audio thread through a seqlock rather than a mutex.
#pragma once

#include "Pedals.h"
#include <atomic>
#include <array>

namespace bassamp {

enum class PedalPlacement
{
    FrontOfAmp = 0,   // between bass and amp input - where dirt and filters belong
    AmpLoop,          // after the preamp, before the cabinet
    PostCab,          // after the speaker - where a studio would put ambience
    NumPlacements
};

const char* placementName (PedalPlacement p);

class Pedalboard
{
public:
    static constexpr int kNumSlots = (int) PedalType::NumTypes;

    Pedalboard();

    void prepare (double sampleRate, int maxBlockSize);
    void reset();
    void setTempo (double bpm);

    void process (float* data, int numSamples, PedalPlacement where);

    // --- message thread -----------------------------------------------------
    int    numSlots() const { return kNumSlots; }
    Pedal* pedalAt (int chainIndex);                 // in current chain order
    Pedal* pedalOfType (PedalType type);
    int    chainIndexOfType (PedalType type) const;

    PedalPlacement placementOf (PedalType type) const;
    void setPlacement (PedalType type, PedalPlacement placement);
    void moveTo (PedalType type, int newChainIndex);
    void restoreDefaultOrder();

    // Chain order as pedal type indices, front of chain first.
    std::array<int, kNumSlots> getOrder() const;
    void setOrder (const std::array<int, kNumSlots>& order);

private:
    struct ChainConfig
    {
        int     order[kNumSlots] { };
        uint8_t placement[kNumSlots] { };
    };

    void publish (const ChainConfig& cfg);
    ChainConfig readConfig() const;

    PedalPtr pedals[kNumSlots];

    ChainConfig shared;                       // written under the seqlock
    ChainConfig editing;                      // message-thread working copy
    mutable ChainConfig audioCopy;            // last good copy seen by the audio thread
    std::atomic<uint32_t> sequence { 0 };
};

} // namespace bassamp
