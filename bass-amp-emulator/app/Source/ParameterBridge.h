// ParameterBridge.h - maps the host's parameter tree onto the DSP rig.
//
// The parameter tree is the single source of truth. Presets, the UI and host
// automation all write parameters; the audio thread reads them once per block
// and pushes them into the rig. Nothing writes the rig directly, which is what
// stops a preset load and an automation lane fighting each other.
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "bassamp/Presets.h"

class ParameterBridge
{
public:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    void connect (juce::AudioProcessorValueTreeState& state);

    // Audio thread, once per block.
    void applyToRig (bassamp::Rig& rig);

    // Message thread: writes a preset into the parameters (and the chain layout,
    // which is structure rather than automation, straight into the board).
    static void applyPresetToParameters (juce::AudioProcessorValueTreeState& state,
                                         bassamp::Rig& rig,
                                         const bassamp::Preset& preset);

    static juce::String pedalParamId (int pedalIndex, int paramIndex);
    static juce::String pedalEnabledId (int pedalIndex);
    static juce::String pedalPlacementId (int pedalIndex);

private:
    struct AmpPointers
    {
        std::atomic<float>* type = nullptr;
        std::atomic<float>* gain = nullptr;
        std::atomic<float>* master = nullptr;
        std::atomic<float>* output = nullptr;
        std::atomic<float>* bass = nullptr;
        std::atomic<float>* mid = nullptr;
        std::atomic<float>* treble = nullptr;
        std::atomic<float>* midFreq = nullptr;
        std::atomic<float>* presence = nullptr;
        std::atomic<float>* blend = nullptr;
        std::atomic<float>* sub = nullptr;
        std::atomic<float>* bright = nullptr;
        std::atomic<float>* ultraLo = nullptr;
        std::atomic<float>* ultraHi = nullptr;
        std::atomic<float>* shape = nullptr;
        std::atomic<float>* sag = nullptr;
    };

    struct CabPointers
    {
        std::atomic<float>* type = nullptr;
        std::atomic<float>* mic = nullptr;
        std::atomic<float>* position = nullptr;
        std::atomic<float>* distance = nullptr;
        std::atomic<float>* room = nullptr;
        std::atomic<float>* horn = nullptr;
        std::atomic<float>* lowCut = nullptr;
        std::atomic<float>* highCut = nullptr;
    };

    struct RigPointers
    {
        std::atomic<float>* input = nullptr;
        std::atomic<float>* master = nullptr;
        std::atomic<float>* di = nullptr;
    };

    struct PedalPointers
    {
        std::atomic<float>* enabled = nullptr;
        std::atomic<float>* placement = nullptr;
        std::atomic<float>* params[bassamp::Pedal::kMaxParams] { };
        int numParams = 0;
    };

    AmpPointers amp;
    CabPointers cab;
    RigPointers rigPtrs;
    PedalPointers pedals[(int) bassamp::PedalType::NumTypes];

    bassamp::CabinetSettings lastCab;
    bool cabInitialised = false;
};
