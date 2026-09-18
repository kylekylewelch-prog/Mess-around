#include "PluginEditor.h"

using namespace bassamp;

BassAmpEditor::BassAmpEditor (BassAmpProcessor& p)
    : juce::AudioProcessorEditor (&p),
      processor (p),
      inputMeter  ([&p] { return p.getRig().getInputLevel(); },
                   [&p] { return p.getRig().isInputClipping(); }),
      outputMeter ([&p] { return p.getRig().getOutputLevel(); }),
      ampPanel (p), pedalPanel (p), cabPanel (p), tunerPanel (p),
      metronomePanel (p), trackPanel (p), settingsPanel (p)
{
    setLookAndFeel (&lookAndFeel);

    categoryFilter.addItem ("All genres", 1);
    int id = 2;
    for (const auto& c : presetCategories()) categoryFilter.addItem (c, id++);
    categoryFilter.setSelectedId (1, juce::dontSendNotification);
    categoryFilter.onChange = [this] { rebuildPresetList(); };
    addAndMakeVisible (categoryFilter);

    presetSelect.onChange = [this] { loadSelectedPreset(); };
    addAndMakeVisible (presetSelect);

    previousPreset.onClick = [this]
    {
        const int index = presetSelect.getSelectedItemIndex();
        if (index > 0) presetSelect.setSelectedItemIndex (index - 1);
    };
    nextPreset.onClick = [this]
    {
        const int index = presetSelect.getSelectedItemIndex();
        if (index < presetSelect.getNumItems() - 1) presetSelect.setSelectedItemIndex (index + 1);
    };
    addAndMakeVisible (previousPreset);
    addAndMakeVisible (nextPreset);

    savePreset.onClick = [this] { savePresetFile(); };
    loadPreset.onClick = [this] { loadPresetFile(); };
    addAndMakeVisible (savePreset);
    addAndMakeVisible (loadPreset);

    presetNotes.setFont (juce::Font (juce::FontOptions (11.5f)));
    presetNotes.setColour (juce::Label::textColourId, theme::textDim);
    presetNotes.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (presetNotes);

    addAndMakeVisible (inputMeter);
    addAndMakeVisible (outputMeter);
    for (auto* l : { &inputLabel, &outputLabel })
    {
        l->setFont (juce::Font (juce::FontOptions (10.5f)));
        l->setColour (juce::Label::textColourId, theme::textDim);
        addAndMakeVisible (*l);
    }
    inputLabel.setText ("IN", juce::dontSendNotification);
    outputLabel.setText ("OUT", juce::dontSendNotification);

    tabs.setOutline (0);
    tabs.setTabBarDepth (32);
    tabs.addTab ("AMP",       theme::background, &ampPanel,       false);
    tabs.addTab ("PEDALS",    theme::background, &pedalPanel,     false);
    tabs.addTab ("CABINET",   theme::background, &cabPanel,       false);
    tabs.addTab ("TUNER",     theme::background, &tunerPanel,     false);
    tabs.addTab ("METRONOME", theme::background, &metronomePanel, false);
    tabs.addTab ("TRACK",     theme::background, &trackPanel,     false);
    tabs.addTab ("SETUP",     theme::background, &settingsPanel,  false);
    addAndMakeVisible (tabs);

    rebuildPresetList();

    setResizable (true, true);
    setResizeLimits (960, 620, 2200, 1400);
    setSize (1120, 720);

    startTimerHz (2);
}

BassAmpEditor::~BassAmpEditor()
{
    setLookAndFeel (nullptr);
}

void BassAmpEditor::rebuildPresetList()
{
    const juce::String category = categoryFilter.getSelectedId() == 1
                                    ? juce::String()
                                    : categoryFilter.getText();

    presetSelect.clear (juce::dontSendNotification);
    for (int i = 0; i < numPresets(); ++i)
    {
        const auto& preset = getPreset (i);
        if (category.isNotEmpty() && juce::String (preset.category) != category) continue;
        // Item IDs carry the preset index so filtering never renumbers anything.
        presetSelect.addItem (preset.name, i + 1);
    }

    if (presetSelect.getNumItems() > 0) presetSelect.setSelectedItemIndex (0, juce::dontSendNotification);
    loadSelectedPreset();
}

void BassAmpEditor::loadSelectedPreset()
{
    const int index = presetSelect.getSelectedId() - 1;
    if (index < 0 || index >= numPresets()) return;

    processor.loadPreset (index);

    const auto& preset = getPreset (index);
    presetNotes.setText (juce::String (preset.reference) + "   |   " + juce::String (preset.notes),
                         juce::dontSendNotification);
    presetNotes.setTooltip (juce::String (preset.reference) + "\n\n" + juce::String (preset.notes));
}

void BassAmpEditor::savePresetFile()
{
    chooser = std::make_unique<juce::FileChooser> (
        "Save rig", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                        .getChildFile ("BassAmp Rigs"), "*.bassrig");

    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                          | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file == juce::File()) return;
        if (file.getFileExtension().isEmpty()) file = file.withFileExtension ("bassrig");
        file.getParentDirectory().createDirectory();

        if (! file.replaceWithText (processor.getRig().saveState()))
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                    "Could not save", "Writing " + file.getFullPathName() + " failed.");
    });
}

void BassAmpEditor::loadPresetFile()
{
    chooser = std::make_unique<juce::FileChooser> (
        "Open rig", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                        .getChildFile ("BassAmp Rigs"), "*.bassrig");

    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this] (const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();
        if (file == juce::File()) return;

        if (! processor.getRig().loadState (file.loadFileAsString().toStdString()))
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                    "Could not open", "That does not look like a saved rig file.");
            return;
        }

        // The rig is the source of truth for a file load, so push it back into
        // the parameter tree or the next block would overwrite it.
        auto& state = processor.getState();
        const auto& ampSettings = processor.getRig().amp().getSettings();
        auto push = [&state] (const juce::String& id, float value)
        {
            if (auto* param = state.getParameter (id))
                param->setValueNotifyingHost (param->convertTo0to1 (value));
        };

        push ("amp_type", (float) (int) ampSettings.type);
        push ("amp_gain", ampSettings.gain);
        push ("amp_master", ampSettings.master);
        push ("amp_output", ampSettings.output);
        push ("amp_bass", ampSettings.bass);
        push ("amp_mid", ampSettings.mid);
        push ("amp_treble", ampSettings.treble);
        push ("amp_midfreq", (float) ampSettings.midFreqIndex);
        push ("amp_presence", ampSettings.presence);
        push ("amp_blend", ampSettings.blend);
        push ("amp_sub", ampSettings.subLevel);
        push ("amp_bright", ampSettings.bright ? 1.0f : 0.0f);
        push ("amp_ultralo", ampSettings.ultraLo ? 1.0f : 0.0f);
        push ("amp_ultrahi", ampSettings.ultraHi ? 1.0f : 0.0f);
        push ("amp_shape", ampSettings.shape ? 1.0f : 0.0f);
        push ("amp_sag", ampSettings.sag);

        const auto& cabSettings = processor.getRig().getCabinetSettings();
        push ("cab_type", (float) (int) cabSettings.cab);
        push ("cab_mic", (float) (int) cabSettings.mic);
        push ("cab_position", cabSettings.micPosition);
        push ("cab_distance", cabSettings.micDistance);
        push ("cab_room", cabSettings.roomAmount);
        push ("cab_horn", cabSettings.hornEnabled ? 1.0f : 0.0f);
        push ("cab_lowcut", cabSettings.lowCutHz);
        push ("cab_highcut", cabSettings.highCutHz);

        const auto& rigSettings = processor.getRig().getSettings();
        push ("rig_input", rigSettings.inputGainDb);
        push ("rig_master", rigSettings.masterLevelDb);
        push ("rig_di", rigSettings.diBlend);

        auto& board = processor.getRig().pedalboard();
        for (int i = 0; i < Pedalboard::kNumSlots; ++i)
        {
            auto* pedal = board.pedalOfType ((PedalType) i);
            if (pedal == nullptr) continue;
            push (ParameterBridge::pedalEnabledId (i), pedal->isEnabled() ? 1.0f : 0.0f);
            push (ParameterBridge::pedalPlacementId (i), (float) (int) board.placementOf ((PedalType) i));
            for (int q = 0; q < pedal->numParams(); ++q)
                push (ParameterBridge::pedalParamId (i, q), pedal->getParamValue (q));
        }

        presetNotes.setText ("Loaded " + file.getFileName(), juce::dontSendNotification);
    });
}

void BassAmpEditor::timerCallback()
{
    // Nothing time critical here - the panels run their own timers. This just
    // keeps the preset notes line honest if the host changed program.
    const int program = processor.getCurrentProgram();
    if (program >= 0 && program < numPresets() && presetSelect.getSelectedId() - 1 != program)
        presetSelect.setSelectedId (program + 1, juce::dontSendNotification);
}

void BassAmpEditor::paint (juce::Graphics& g)
{
    g.fillAll (theme::background);

    auto header = getLocalBounds().removeFromTop (76);
    g.setColour (theme::panel);
    g.fillRect (header);
    g.setColour (theme::outline);
    g.drawHorizontalLine (header.getBottom() - 1, 0.0f, (float) getWidth());

    g.setColour (theme::accent);
    g.setFont (juce::Font (juce::FontOptions (19.0f, juce::Font::bold)));
    g.drawText ("BASSAMP STUDIO", header.removeFromLeft (200).reduced (14, 0),
                juce::Justification::centredLeft, false);
}

void BassAmpEditor::resized()
{
    auto bounds = getLocalBounds();

    auto header = bounds.removeFromTop (76).reduced (14, 8);
    header.removeFromLeft (190);

    auto topRow = header.removeFromTop (28);

    auto meters = topRow.removeFromRight (190);
    inputLabel.setBounds (meters.removeFromLeft (24));
    inputMeter.setBounds (meters.removeFromLeft (64).reduced (0, 8));
    meters.removeFromLeft (6);
    outputLabel.setBounds (meters.removeFromLeft (30));
    outputMeter.setBounds (meters.reduced (0, 8));

    categoryFilter.setBounds (topRow.removeFromLeft (150));
    topRow.removeFromLeft (8);
    previousPreset.setBounds (topRow.removeFromLeft (28));
    presetSelect.setBounds (topRow.removeFromLeft (230));
    nextPreset.setBounds (topRow.removeFromLeft (28));
    topRow.removeFromLeft (12);
    savePreset.setBounds (topRow.removeFromLeft (80));
    topRow.removeFromLeft (6);
    loadPreset.setBounds (topRow.removeFromLeft (80));

    header.removeFromTop (4);
    presetNotes.setBounds (header);

    tabs.setBounds (bounds.reduced (10, 6));
}
