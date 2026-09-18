#include "LookAndFeel.h"

BassAmpLookAndFeel::BassAmpLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, theme::background);
    setColour (juce::Label::textColourId, theme::text);
    setColour (juce::ComboBox::backgroundColourId, theme::panelLight);
    setColour (juce::ComboBox::textColourId, theme::text);
    setColour (juce::ComboBox::outlineColourId, theme::outline);
    setColour (juce::ComboBox::arrowColourId, theme::textDim);
    setColour (juce::PopupMenu::backgroundColourId, theme::panel);
    setColour (juce::PopupMenu::textColourId, theme::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, theme::accentDim);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextButton::buttonColourId, theme::panelLight);
    setColour (juce::TextButton::textColourOffId, theme::text);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    setColour (juce::ScrollBar::thumbColourId, theme::outline);
    setColour (juce::TooltipWindow::backgroundColourId, theme::panel);
    setColour (juce::TooltipWindow::textColourId, theme::text);
    setColour (juce::TooltipWindow::outlineColourId, theme::outline);
}

void BassAmpLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                           juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const float trackWidth = juce::jmax (2.0f, radius * 0.16f);

    // Track
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius - trackWidth, radius - trackWidth,
                         0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (theme::outline);
    g.strokePath (track, juce::PathStrokeType (trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value arc. Bipolar controls fill from the centre, which makes a cut versus
    // a boost readable at a glance.
    const bool bipolar = slider.getMinimum() < -0.001 && slider.getMaximum() > 0.001;
    const float originPos = bipolar ? (float) slider.valueToProportionOfLength (0.0) : 0.0f;
    const float originAngle = rotaryStartAngle + originPos * (rotaryEndAngle - rotaryStartAngle);

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, radius - trackWidth, radius - trackWidth, 0.0f,
                         juce::jmin (originAngle, angle), juce::jmax (originAngle, angle), true);
    g.setColour (slider.isEnabled() ? theme::accent : theme::accentDim.withAlpha (0.4f));
    g.strokePath (value, juce::PathStrokeType (trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Knob body
    const float knobRadius = radius - trackWidth * 2.0f;
    juce::ColourGradient body (theme::panelLight.brighter (0.25f), centre.x, centre.y - knobRadius,
                               theme::panel.darker (0.4f), centre.x, centre.y + knobRadius, false);
    g.setGradientFill (body);
    g.fillEllipse (juce::Rectangle<float> (knobRadius * 2.0f, knobRadius * 2.0f).withCentre (centre));
    g.setColour (theme::outline);
    g.drawEllipse (juce::Rectangle<float> (knobRadius * 2.0f, knobRadius * 2.0f).withCentre (centre), 1.0f);

    // Pointer
    juce::Path pointer;
    const float pointerLength = knobRadius * 0.72f;
    const float pointerThickness = juce::jmax (2.0f, knobRadius * 0.14f);
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -knobRadius * 0.92f,
                                 pointerThickness, pointerLength, pointerThickness * 0.5f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre));
    g.setColour (slider.isEnabled() ? theme::text : theme::textDim);
    g.fillPath (pointer);
}

void BassAmpLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                           bool isHighlighted, bool)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const bool on = button.getToggleState();

    // Footswitch-style: a rounded pad with an LED.
    g.setColour (on ? theme::accentDim.withAlpha (0.35f) : theme::panelLight);
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (on ? theme::accent : (isHighlighted ? theme::textDim : theme::outline));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    auto led = bounds.removeFromLeft (bounds.getHeight()).reduced (bounds.getHeight() * 0.32f);
    g.setColour (on ? theme::accent : theme::outline);
    g.fillEllipse (led);
    if (on)
    {
        g.setColour (theme::accent.withAlpha (0.25f));
        g.fillEllipse (led.expanded (2.5f));
    }

    g.setColour (on ? theme::text : theme::textDim);
    g.setFont (juce::Font (juce::FontOptions (13.0f)));
    g.drawText (button.getButtonText(), bounds.reduced (4.0f, 0.0f), juce::Justification::centredLeft, true);
}

void BassAmpLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                               const juce::Colour& backgroundColour,
                                               bool isHighlighted, bool isDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    auto base = backgroundColour;
    if (isDown)            base = base.brighter (0.25f);
    else if (isHighlighted) base = base.brighter (0.12f);

    g.setColour (base);
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (theme::outline);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

void BassAmpLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                       int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (theme::outline);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    juce::Path arrow;
    const float cx = (float) width - 14.0f, cy = (float) height * 0.5f;
    arrow.startNewSubPath (cx - 4.0f, cy - 2.0f);
    arrow.lineTo (cx, cy + 3.0f);
    arrow.lineTo (cx + 4.0f, cy - 2.0f);
    g.setColour (theme::textDim);
    g.strokePath (arrow, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void BassAmpLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float, float,
                                           juce::Slider::SliderStyle style, juce::Slider& slider)
{
    const bool vertical = style == juce::Slider::LinearVertical;
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();

    if (vertical)
    {
        auto track = bounds.withSizeKeepingCentre (5.0f, bounds.getHeight());
        g.setColour (theme::outline);
        g.fillRoundedRectangle (track, 2.5f);

        const float zero = (float) slider.valueToProportionOfLength (0.0);
        const float zeroY = bounds.getBottom() - zero * bounds.getHeight();
        g.setColour (theme::accent);
        g.fillRoundedRectangle (track.withTop (juce::jmin (sliderPos, zeroY))
                                     .withBottom (juce::jmax (sliderPos, zeroY)), 2.5f);

        auto thumb = juce::Rectangle<float> (bounds.getWidth() - 4.0f, 10.0f)
                        .withCentre ({ bounds.getCentreX(), sliderPos });
        g.setColour (theme::panelLight.brighter (0.3f));
        g.fillRoundedRectangle (thumb, 3.0f);
        g.setColour (theme::outline);
        g.drawRoundedRectangle (thumb, 3.0f, 1.0f);
    }
    else
    {
        auto track = bounds.withSizeKeepingCentre (bounds.getWidth(), 5.0f);
        g.setColour (theme::outline);
        g.fillRoundedRectangle (track, 2.5f);
        g.setColour (theme::accent);
        g.fillRoundedRectangle (track.withRight (sliderPos), 2.5f);

        auto thumb = juce::Rectangle<float> (10.0f, bounds.getHeight() - 4.0f)
                        .withCentre ({ sliderPos, bounds.getCentreY() });
        g.setColour (theme::panelLight.brighter (0.3f));
        g.fillRoundedRectangle (thumb, 3.0f);
        g.setColour (theme::outline);
        g.drawRoundedRectangle (thumb, 3.0f, 1.0f);
    }
}

juce::Font BassAmpLookAndFeel::getLabelFont (juce::Label& label)
{
    return juce::Font (juce::FontOptions (label.getFont().getHeight()));
}

juce::Font BassAmpLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions (13.5f));
}
