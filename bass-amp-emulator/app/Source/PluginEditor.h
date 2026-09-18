#pragma once

#include "PluginProcessor.h"
#include "Gui/LookAndFeel.h"
#include "Gui/Panels.h"
#include "Gui/UtilityPanels.h"

class BassAmpEditor : public juce::AudioProcessorEditor,
                      private juce::Timer
{
public:
    explicit BassAmpEditor (BassAmpProcessor&);
    ~BassAmpEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void rebuildPresetList();
    void loadSelectedPreset();
    void savePresetFile();
    void loadPresetFile();

    BassAmpProcessor& processor;
    BassAmpLookAndFeel lookAndFeel;

    juce::ComboBox categoryFilter, presetSelect;
    juce::TextButton previousPreset { "<" }, nextPreset { ">" };
    juce::TextButton savePreset { "Save..." }, loadPreset { "Open..." };
    juce::Label presetNotes;

    LevelMeterComponent inputMeter, outputMeter;
    juce::Label inputLabel, outputLabel;

    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    AmpPanel ampPanel;
    PedalboardPanel pedalPanel;
    CabPanel cabPanel;
    TunerPanel tunerPanel;
    MetronomePanel metronomePanel;
    TrackPanel trackPanel;
    SettingsPanel settingsPanel;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::TooltipWindow tooltips { this, 600 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassAmpEditor)
};
