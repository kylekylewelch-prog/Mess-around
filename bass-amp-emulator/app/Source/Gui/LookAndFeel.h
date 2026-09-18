#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace theme
{
    const juce::Colour background   { 0xff16181c };
    const juce::Colour panel        { 0xff1e2127 };
    const juce::Colour panelLight   { 0xff262a32 };
    const juce::Colour outline      { 0xff33383f };
    const juce::Colour text         { 0xffd8dde4 };
    const juce::Colour textDim      { 0xff8a919c };
    const juce::Colour accent       { 0xffe0a458 };   // warm amber, like a valve amp's pilot lamp
    const juce::Colour accentDim    { 0xff8a6535 };
    const juce::Colour good         { 0xff5fbf7f };
    const juce::Colour warn         { 0xffd9534f };
    const juce::Colour blue         { 0xff5a9fd4 };
}

class BassAmpLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BassAmpLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
};
