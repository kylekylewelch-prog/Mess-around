#include "ParameterBridge.h"

using namespace bassamp;
using APVTS = juce::AudioProcessorValueTreeState;

namespace {

std::unique_ptr<juce::AudioParameterFloat> floatParam (const juce::String& id, const juce::String& name,
                                                       float min, float max, float def,
                                                       float skew = 0.5f, const juce::String& unit = {})
{
    juce::NormalisableRange<float> range (min, max);
    if (skew > 0.0f && std::abs (skew - 0.5f) > 0.001f && min < max)
        range.setSkewForCentre (min + (max - min) * skew);

    return std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id, 1 }, name, range, def,
        juce::AudioParameterFloatAttributes().withLabel (unit));
}

std::unique_ptr<juce::AudioParameterChoice> choiceParam (const juce::String& id, const juce::String& name,
                                                         juce::StringArray choices, int def)
{
    return std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { id, 1 }, name, choices, def);
}

std::unique_ptr<juce::AudioParameterBool> boolParam (const juce::String& id, const juce::String& name, bool def)
{
    return std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id, 1 }, name, def);
}

} // namespace

juce::String ParameterBridge::pedalParamId (int pedalIndex, int paramIndex)
{
    return "p" + juce::String (pedalIndex) + "_" + juce::String (paramIndex);
}

juce::String ParameterBridge::pedalEnabledId (int pedalIndex)
{
    return "p" + juce::String (pedalIndex) + "_on";
}

juce::String ParameterBridge::pedalPlacementId (int pedalIndex)
{
    return "p" + juce::String (pedalIndex) + "_place";
}

APVTS::ParameterLayout ParameterBridge::createLayout()
{
    APVTS::ParameterLayout layout;

    // --- amp ---------------------------------------------------------------
    juce::StringArray ampNames;
    for (int i = 0; i < (int) AmpType::NumTypes; ++i) ampNames.add (ampName ((AmpType) i));

    layout.add (choiceParam ("amp_type", "Amp", ampNames, (int) AmpType::AmpegSvt));
    layout.add (floatParam ("amp_gain",     "Amp Gain",     0.0f, 1.0f, 0.5f));
    layout.add (floatParam ("amp_master",   "Amp Master",   0.0f, 1.0f, 0.6f));
    layout.add (floatParam ("amp_output",   "Amp Output",   0.0f, 1.0f, 0.7f));
    layout.add (floatParam ("amp_bass",     "Bass",         0.0f, 1.0f, 0.5f));
    layout.add (floatParam ("amp_mid",      "Mid",          0.0f, 1.0f, 0.5f));
    layout.add (floatParam ("amp_treble",   "Treble",       0.0f, 1.0f, 0.5f));
    layout.add (choiceParam ("amp_midfreq", "Mid Frequency", { "1", "2", "3", "4", "5" }, 2));
    layout.add (floatParam ("amp_presence", "Presence",     0.0f, 1.0f, 0.3f));
    layout.add (floatParam ("amp_blend",    "DI Drive Blend", 0.0f, 1.0f, 1.0f));
    layout.add (floatParam ("amp_sub",      "Sub Harmonic", 0.0f, 1.0f, 0.0f));
    layout.add (boolParam  ("amp_bright",   "Bright",  false));
    layout.add (boolParam  ("amp_ultralo",  "Ultra Lo", false));
    layout.add (boolParam  ("amp_ultrahi",  "Ultra Hi", false));
    layout.add (boolParam  ("amp_shape",    "Shape",   false));
    layout.add (floatParam ("amp_sag",      "Supply Sag", 0.0f, 1.0f, 0.5f));

    // --- cabinet -----------------------------------------------------------
    juce::StringArray cabNames;
    for (int i = 0; i < (int) CabType::NumTypes; ++i) cabNames.add (cabName ((CabType) i));
    juce::StringArray micNames;
    for (int i = 0; i < (int) MicType::NumTypes; ++i) micNames.add (micName ((MicType) i));

    layout.add (choiceParam ("cab_type", "Cabinet", cabNames, (int) CabType::C4x10));
    layout.add (choiceParam ("cab_mic",  "Microphone", micNames, (int) MicType::DynamicLarge));
    layout.add (floatParam ("cab_position", "Mic Position", 0.0f, 1.0f, 0.35f));
    layout.add (floatParam ("cab_distance", "Mic Distance", 0.0f, 1.0f, 0.30f));
    layout.add (floatParam ("cab_room",     "Room",         0.0f, 1.0f, 0.15f));
    layout.add (boolParam  ("cab_horn",     "Horn",         false));
    layout.add (floatParam ("cab_lowcut",   "Cab Low Cut",  20.0f, 200.0f, 30.0f, 0.35f, "Hz"));
    layout.add (floatParam ("cab_highcut",  "Cab High Cut", 2000.0f, 19000.0f, 12000.0f, 0.35f, "Hz"));

    // --- rig ---------------------------------------------------------------
    layout.add (floatParam ("rig_input",  "Input Gain",   -24.0f, 24.0f, 0.0f, 0.5f, "dB"));
    layout.add (floatParam ("rig_master", "Master Level", -60.0f, 12.0f, -3.0f, 0.7f, "dB"));
    layout.add (floatParam ("rig_di",     "DI Blend",     0.0f, 1.0f, 0.0f));

    // --- pedals ------------------------------------------------------------
    // Built straight from the pedal descriptors, so a new pedal needs no changes
    // here at all.
    for (int i = 0; i < (int) PedalType::NumTypes; ++i)
    {
        auto pedal = createPedal ((PedalType) i);
        if (pedal == nullptr) continue;

        const juce::String prefix = juce::String (pedalTypeName ((PedalType) i)) + ": ";

        layout.add (boolParam (pedalEnabledId (i), prefix + "On", false));
        layout.add (choiceParam (pedalPlacementId (i), prefix + "Position",
                                 { "Front of amp", "Amp FX loop", "After cab" }, 0));

        for (int q = 0; q < pedal->numParams(); ++q)
        {
            const auto& d = pedal->param (q);
            if (d.numChoices > 0)
            {
                juce::StringArray choices;
                for (int c = 0; c < d.numChoices; ++c)
                    choices.add (d.choices != nullptr ? d.choices[c] : juce::String (c + 1));
                layout.add (choiceParam (pedalParamId (i, q), prefix + d.name, choices,
                                         (int) d.defaultValue));
            }
            else
            {
                layout.add (floatParam (pedalParamId (i, q), prefix + d.name,
                                        d.minValue, d.maxValue, d.defaultValue, d.skew, d.unit));
            }
        }
    }

    return layout;
}

void ParameterBridge::connect (APVTS& state)
{
    auto get = [&state] (const juce::String& id) { return state.getRawParameterValue (id); };

    amp.type = get ("amp_type");   amp.gain = get ("amp_gain");
    amp.master = get ("amp_master"); amp.output = get ("amp_output");
    amp.bass = get ("amp_bass");   amp.mid = get ("amp_mid");
    amp.treble = get ("amp_treble"); amp.midFreq = get ("amp_midfreq");
    amp.presence = get ("amp_presence"); amp.blend = get ("amp_blend");
    amp.sub = get ("amp_sub");     amp.bright = get ("amp_bright");
    amp.ultraLo = get ("amp_ultralo"); amp.ultraHi = get ("amp_ultrahi");
    amp.shape = get ("amp_shape"); amp.sag = get ("amp_sag");

    cab.type = get ("cab_type");   cab.mic = get ("cab_mic");
    cab.position = get ("cab_position"); cab.distance = get ("cab_distance");
    cab.room = get ("cab_room");   cab.horn = get ("cab_horn");
    cab.lowCut = get ("cab_lowcut"); cab.highCut = get ("cab_highcut");

    rigPtrs.input = get ("rig_input");
    rigPtrs.master = get ("rig_master");
    rigPtrs.di = get ("rig_di");

    for (int i = 0; i < (int) PedalType::NumTypes; ++i)
    {
        pedals[i].enabled = get (pedalEnabledId (i));
        pedals[i].placement = get (pedalPlacementId (i));

        auto probe = createPedal ((PedalType) i);
        pedals[i].numParams = probe != nullptr ? probe->numParams() : 0;
        for (int q = 0; q < pedals[i].numParams; ++q)
            pedals[i].params[q] = get (pedalParamId (i, q));
    }
}

void ParameterBridge::applyToRig (Rig& rig)
{
    AmpSettings a;
    a.type   = (AmpType) (int) amp.type->load();
    a.gain   = amp.gain->load();
    a.master = amp.master->load();
    a.output = amp.output->load();
    a.bass   = amp.bass->load();
    a.mid    = amp.mid->load();
    a.treble = amp.treble->load();
    a.midFreqIndex = (int) amp.midFreq->load();
    a.presence = amp.presence->load();
    a.blend  = amp.blend->load();
    a.subLevel = amp.sub->load();
    a.bright = amp.bright->load() >= 0.5f;
    a.ultraLo = amp.ultraLo->load() >= 0.5f;
    a.ultraHi = amp.ultraHi->load() >= 0.5f;
    a.shape  = amp.shape->load() >= 0.5f;
    a.sag    = amp.sag->load();
    rig.amp().setSettings (a);

    RigSettings r = rig.getSettings();
    r.inputGainDb   = rigPtrs.input->load();
    r.masterLevelDb = rigPtrs.master->load();
    r.diBlend       = rigPtrs.di->load();
    rig.setSettings (r);

    // Cabinet changes are expensive (an IR design plus a crossfade), so only
    // request one when something actually moved.
    CabinetSettings c;
    c.cab = (CabType) (int) cab.type->load();
    c.mic = (MicType) (int) cab.mic->load();
    c.micPosition = cab.position->load();
    c.micDistance = cab.distance->load();
    c.roomAmount  = cab.room->load();
    c.hornEnabled = cab.horn->load() >= 0.5f;
    c.lowCutHz    = cab.lowCut->load();
    c.highCutHz   = cab.highCut->load();

    if (! cabInitialised || c != lastCab)
    {
        lastCab = c;
        cabInitialised = true;
        rig.requestCabinetSettings (c);   // the message thread services it
    }

    auto& board = rig.pedalboard();
    for (int i = 0; i < (int) PedalType::NumTypes; ++i)
    {
        Pedal* p = board.pedalOfType ((PedalType) i);
        if (p == nullptr || pedals[i].enabled == nullptr) continue;

        p->setEnabled (pedals[i].enabled->load() >= 0.5f);
        for (int q = 0; q < pedals[i].numParams; ++q)
            if (pedals[i].params[q] != nullptr) p->setParamValue (q, pedals[i].params[q]->load());
    }
}

void ParameterBridge::applyPresetToParameters (APVTS& state, Rig& rig, const Preset& preset)
{
    auto setParam = [&state] (const juce::String& id, float value)
    {
        if (auto* p = state.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (value));
    };

    setParam ("amp_type",     (float) (int) preset.amp.type);
    setParam ("amp_gain",     preset.amp.gain);
    setParam ("amp_master",   preset.amp.master);
    setParam ("amp_output",   preset.amp.output);
    setParam ("amp_bass",     preset.amp.bass);
    setParam ("amp_mid",      preset.amp.mid);
    setParam ("amp_treble",   preset.amp.treble);
    setParam ("amp_midfreq",  (float) preset.amp.midFreqIndex);
    setParam ("amp_presence", preset.amp.presence);
    setParam ("amp_blend",    preset.amp.blend);
    setParam ("amp_sub",      preset.amp.subLevel);
    setParam ("amp_bright",   preset.amp.bright ? 1.0f : 0.0f);
    setParam ("amp_ultralo",  preset.amp.ultraLo ? 1.0f : 0.0f);
    setParam ("amp_ultrahi",  preset.amp.ultraHi ? 1.0f : 0.0f);
    setParam ("amp_shape",    preset.amp.shape ? 1.0f : 0.0f);
    setParam ("amp_sag",      preset.amp.sag);

    setParam ("cab_type",     (float) (int) preset.cab.cab);
    setParam ("cab_mic",      (float) (int) preset.cab.mic);
    setParam ("cab_position", preset.cab.micPosition);
    setParam ("cab_distance", preset.cab.micDistance);
    setParam ("cab_room",     preset.cab.roomAmount);
    setParam ("cab_horn",     preset.cab.hornEnabled ? 1.0f : 0.0f);
    setParam ("cab_lowcut",   preset.cab.lowCutHz);
    setParam ("cab_highcut",  preset.cab.highCutHz);

    setParam ("rig_input",    preset.rig.inputGainDb);
    setParam ("rig_master",   preset.rig.masterLevelDb);
    setParam ("rig_di",       preset.rig.diBlend);

    // Everything off and back to defaults first, then apply what the preset uses.
    for (int i = 0; i < (int) PedalType::NumTypes; ++i)
    {
        auto probe = createPedal ((PedalType) i);
        if (probe == nullptr) continue;
        setParam (pedalEnabledId (i), 0.0f);
        setParam (pedalPlacementId (i), 0.0f);
        for (int q = 0; q < probe->numParams(); ++q)
            setParam (pedalParamId (i, q), probe->param (q).defaultValue);
    }

    for (const auto& pp : preset.pedals)
    {
        const int i = (int) pp.type;
        setParam (pedalEnabledId (i), 1.0f);
        setParam (pedalPlacementId (i), (float) (int) pp.placement);
        for (int q = 0; q < (int) pp.params.size(); ++q)
            setParam (pedalParamId (i, q), pp.params[(size_t) q]);
    }

    // Chain order and placement are layout, not automation, so they go straight
    // to the board.
    auto& board = rig.pedalboard();
    board.restoreDefaultOrder();
    for (int i = 0; i < (int) PedalType::NumTypes; ++i)
        board.setPlacement ((PedalType) i, PedalPlacement::FrontOfAmp);
    for (const auto& pp : preset.pedals)
        board.setPlacement (pp.type, pp.placement);
}
