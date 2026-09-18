// Panels.h - the main screens of the editor.
#pragma once

#include "Components.h"
#include "../PluginProcessor.h"

class AmpPanel : public juce::Component, private juce::Timer
{
public:
    explicit AmpPanel (BassAmpProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshForAmp();

    BassAmpProcessor& processor;
    juce::ComboBox ampSelect;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> ampAttachment;

    std::unique_ptr<Knob> gain, master, output, bass, mid, treble, presence, sag, blend, sub;
    juce::ComboBox midFreq;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> midFreqAttachment;
    juce::Label midFreqLabel;

    juce::ToggleButton bright { "Bright" }, ultraLo { "Ultra Lo" }, ultraHi { "Ultra Hi" }, shape { "Shape" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> brightAtt, ultraLoAtt, ultraHiAtt, shapeAtt;

    ResponseCurve toneCurve;
    juce::Label description;
    juce::Label sagReadout;
    int lastAmpType = -1;
};

class CabPanel : public juce::Component, private juce::Timer
{
public:
    explicit CabPanel (BassAmpProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    BassAmpProcessor& processor;
    juce::ComboBox cabSelect, micSelect;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cabAttachment, micAttachment;
    std::unique_ptr<Knob> position, distance, room, lowCut, highCut;
    juce::ToggleButton horn { "Horn" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hornAtt;
    juce::TextButton loadIr { "Load IR..." }, clearIr { "Use modelled cab" };
    juce::Label irName, description;
    ResponseCurve cabCurve;
    std::unique_ptr<juce::FileChooser> chooser;
    int lastCabType = -1;
};

class PedalboardPanel : public juce::Component, private juce::Timer
{
public:
    explicit PedalboardPanel (BassAmpProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void rebuildList();
    void selectPedal (int pedalTypeIndex);
    void buildControlsFor (int pedalTypeIndex);

    struct Row : public juce::Component
    {
        Row (PedalboardPanel& owner, int typeIndex);
        void paint (juce::Graphics&) override;
        void resized() override;
        void mouseDown (const juce::MouseEvent&) override;

        PedalboardPanel& panel;
        int type;
        juce::ToggleButton power;
        juce::TextButton up { "\xe2\x96\xb2" }, down { "\xe2\x96\xbc" };
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerAtt;
        bool selected = false;
    };

    BassAmpProcessor& processor;
    juce::Viewport listViewport;
    juce::Component listContent;
    juce::OwnedArray<Row> rows;

    juce::Label pedalTitle, pedalDescription;
    juce::ComboBox placement;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> placementAttachment;
    juce::Label placementLabel;
    juce::OwnedArray<Knob> knobs;
    juce::OwnedArray<juce::ComboBox> choiceBoxes;
    juce::OwnedArray<juce::Label> choiceLabels;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> choiceAttachments;
    juce::TextButton resetPedal { "Reset" };

    int selectedPedal = (int) bassamp::PedalType::Compressor;
};
