// Components.h - reusable widgets shared by the panels.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "LookAndFeel.h"
#include "bassamp/Rig.h"
#include <functional>

// A rotary control with its name above and its value below. Attaches straight to
// a parameter by ID.
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& state, const juce::String& paramId,
          const juce::String& displayName)
        : name (displayName)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                    juce::MathConstants<float>::pi * 2.8f, true);
        addAndMakeVisible (slider);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, paramId, slider);
        slider.onValueChange = [this] { repaint(); };
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        g.setColour (isEnabled() ? theme::textDim : theme::textDim.withAlpha (0.4f));
        g.setFont (juce::Font (juce::FontOptions (11.0f)));
        g.drawText (name, bounds.removeFromTop (14), juce::Justification::centred, false);

        g.setColour (isEnabled() ? theme::text : theme::textDim.withAlpha (0.4f));
        g.setFont (juce::Font (juce::FontOptions (11.5f)));
        g.drawText (slider.getTextFromValue (slider.getValue()),
                    bounds.removeFromBottom (14), juce::Justification::centred, false);
    }

    void resized() override { slider.setBounds (getLocalBounds().reduced (0, 14)); }
    void enablementChanged() override { slider.setEnabled (isEnabled()); repaint(); }

    juce::Slider slider;

private:
    juce::String name;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// Horizontal level meter with a peak-hold and a clip indicator.
class LevelMeterComponent : public juce::Component, private juce::Timer
{
public:
    explicit LevelMeterComponent (std::function<float()> source, std::function<bool()> clipSource = {})
        : getLevel (std::move (source)), getClip (std::move (clipSource))
    {
        startTimerHz (30);
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (theme::panel);
        g.fillRoundedRectangle (bounds, 2.0f);

        const float db = bassamp::gainToDb (juce::jmax (1.0e-6f, level));
        const float norm = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);

        auto fill = bounds.reduced (1.0f).withWidth ((bounds.getWidth() - 2.0f) * norm);
        g.setColour (db > -1.0f ? theme::warn : (db > -6.0f ? theme::accent : theme::good));
        g.fillRoundedRectangle (fill, 2.0f);

        if (peakHold > 0.001f)
        {
            const float peakDb = bassamp::gainToDb (peakHold);
            const float peakNorm = juce::jlimit (0.0f, 1.0f, (peakDb + 60.0f) / 60.0f);
            g.setColour (theme::text.withAlpha (0.7f));
            g.fillRect (bounds.getX() + 1.0f + (bounds.getWidth() - 2.0f) * peakNorm, bounds.getY() + 1.0f,
                        1.5f, bounds.getHeight() - 2.0f);
        }

        if (clipping)
        {
            g.setColour (theme::warn);
            g.fillRoundedRectangle (bounds.removeFromRight (5.0f).reduced (1.0f), 1.5f);
        }

        g.setColour (theme::outline);
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 2.0f, 1.0f);
    }

private:
    void timerCallback() override
    {
        level = getLevel ? getLevel() : 0.0f;
        clipping = getClip ? getClip() : false;
        peakHold = juce::jmax (level, peakHold * 0.97f);
        repaint();
    }

    std::function<float()> getLevel;
    std::function<bool()> getClip;
    float level = 0.0f, peakHold = 0.0f;
    bool clipping = false;
};

// Frequency response plot, 20 Hz to 20 kHz on a log axis.
class ResponseCurve : public juce::Component, private juce::Timer
{
public:
    explicit ResponseCurve (std::function<float (float)> magnitudeDb)
        : getMagnitudeDb (std::move (magnitudeDb))
    {
        startTimerHz (12);
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (theme::panel);
        g.fillRoundedRectangle (bounds, 3.0f);

        // Grid
        g.setColour (theme::outline.withAlpha (0.55f));
        static const float gridFreqs[] = { 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f };
        for (float f : gridFreqs)
        {
            const float x = freqToX (f, bounds);
            g.drawVerticalLine ((int) x, bounds.getY(), bounds.getBottom());
        }
        for (int db = -18; db <= 18; db += 6)
        {
            const float y = dbToY ((float) db, bounds);
            g.setColour (db == 0 ? theme::outline : theme::outline.withAlpha (0.35f));
            g.drawHorizontalLine ((int) y, bounds.getX(), bounds.getRight());
        }

        if (! getMagnitudeDb) return;

        juce::Path path;
        const int steps = juce::jmax (2, (int) bounds.getWidth());
        for (int i = 0; i < steps; ++i)
        {
            const float t = (float) i / (float) (steps - 1);
            const float freq = 20.0f * std::pow (1000.0f, t);
            const float y = dbToY (juce::jlimit (-24.0f, 24.0f, getMagnitudeDb (freq)), bounds);
            const float x = bounds.getX() + t * bounds.getWidth();
            if (i == 0) path.startNewSubPath (x, y); else path.lineTo (x, y);
        }

        g.setColour (theme::accent);
        g.strokePath (path, juce::PathStrokeType (1.8f));

        g.setColour (theme::textDim);
        g.setFont (juce::Font (juce::FontOptions (9.5f)));
        g.drawText ("20", bounds.removeFromBottom (11).removeFromLeft (24), juce::Justification::centredLeft, false);
    }

private:
    void timerCallback() override { repaint(); }

    static float freqToX (float freq, juce::Rectangle<float> b)
    {
        return b.getX() + b.getWidth() * std::log (freq / 20.0f) / std::log (1000.0f);
    }
    static float dbToY (float db, juce::Rectangle<float> b)
    {
        return b.getCentreY() - (db / 24.0f) * (b.getHeight() * 0.5f - 4.0f);
    }

    std::function<float (float)> getMagnitudeDb;
};
