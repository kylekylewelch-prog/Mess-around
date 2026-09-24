// Presets.h - the factory preset library.
//
// Presets are defined in code rather than as data files so they are type
// checked, diffable and cannot drift out of sync with the parameter ranges.
// Each one records what rig it is based on, because "which knob did they turn"
// is far more useful than a preset name on its own.
#pragma once

#include "Rig.h"
#include <string>
#include <vector>

namespace bassamp {

struct PedalPreset
{
    PedalType type = PedalType::Compressor;
    PedalPlacement placement = PedalPlacement::FrontOfAmp;
    std::vector<float> params;   // empty leaves the pedal at its defaults
};

struct Preset
{
    std::string name;
    std::string category;    // genre or family
    std::string reference;   // the rig and player this is modelled on
    std::string notes;       // how to play it, what to adjust
    AmpSettings amp;
    CabinetSettings cab;
    RigSettings rig;
    std::vector<PedalPreset> pedals;
};

int            numPresets();
const Preset&  getPreset (int index);
int            findPreset (const std::string& name);
std::vector<std::string> presetCategories();

// Applies a preset to a rig. Pedals not named by the preset are switched off.
void applyPreset (Rig& rig, const Preset& preset);

} // namespace bassamp
