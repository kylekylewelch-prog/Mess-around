// UtilityPanels.h - tuner, metronome, backing track and settings.
#pragma once

#include "Components.h"
#include "../PluginProcessor.h"

// The rotating strobe rings. Each ring tracks one octave of the reference pitch:
// when the note is exactly in tune the pattern stands still, and the direction of
// travel tells you whether you are sharp or flat. The higher rings move at
// multiples of the error, which is what gives a strobe tuner its resolution.
class StrobeDisplay : public juce::Component, private juce::Timer
{
public:
    explicit StrobeDisplay (bassamp::Tuner& t) : tuner (t) { startTimerHz (50); }
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { repaint(); }
    bassamp::Tuner& tuner;
};

class TunerPanel : public juce::Component, private juce::Timer
{
public:
    explicit TunerPanel (BassAmpProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    void visibilityChanged() override;

private:
    void timerCallback() override;
    void rebuildStringButtons();

    BassAmpProcessor& processor;
    StrobeDisplay strobe;
    juce::Label noteLabel, centsLabel, frequencyLabel, hint;
    juce::ComboBox tuningSelect;
    juce::Slider referenceA;
    juce::Label referenceLabel;
    juce::ToggleButton muteWhileTuning { "Mute output while tuning" };
    juce::ToggleButton autoTarget { "Follow played note" };
    juce::OwnedArray<juce::TextButton> stringButtons;
};

class MetronomePanel : public juce::Component, private juce::Timer
{
public:
    explicit MetronomePanel (BassAmpProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void rebuildAccents();

    BassAmpProcessor& processor;
    juce::TextButton startStop { "Start" }, tapButton { "Tap" };
    juce::Slider tempo, swing, level;
    juce::Label tempoLabel, swingLabel, levelLabel, signatureLabel, subdivisionLabel, countInLabel;
    juce::ComboBox numerator, denominator, subdivision, countIn;
    juce::OwnedArray<juce::TextButton> accentButtons;
    int lastBeatsPerBar = 0;
};

class TrackPanel : public juce::Component, private juce::Timer
{
public:
    explicit TrackPanel (BassAmpProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    BassAmpProcessor& processor;
    juce::TextButton loadButton { "Load track..." }, playButton { "Play" }, stopButton { "Stop" };
    juce::ToggleButton loopButton { "Loop" };
    juce::Slider position, level, bassCut;
    juce::Label fileLabel, timeLabel, levelLabel, bassCutLabel, hint;
    std::unique_ptr<juce::FileChooser> chooser;
    bool dragging = false;
};

class SettingsPanel : public juce::Component, private juce::Timer
{
public:
    explicit SettingsPanel (BassAmpProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    BassAmpProcessor& processor;
    juce::TextButton audioSettings { "Audio device settings..." };
    juce::ComboBox oversampling, expressionCc;
    juce::Label oversamplingLabel, expressionLabel, statusLabel, latencyHelp;
    juce::Slider expressionPedal;
    juce::Label expressionPedalLabel;
};
