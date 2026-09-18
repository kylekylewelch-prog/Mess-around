#include "UtilityPanels.h"

#if JucePlugin_Build_Standalone
 #include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#endif

using namespace bassamp;

// ============================================================================
//  Strobe display
// ============================================================================

void StrobeDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (4.0f);
    g.setColour (theme::panelLight);
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (theme::outline);
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    const auto centre = bounds.getCentre();
    const float outer = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.46f;
    const int bands = bassamp::Tuner::kStrobeBands;
    const float ringThickness = outer / (float) (bands + 1);
    const bool locked = tuner.hasSignal();

    for (int b = 0; b < bands; ++b)
    {
        const float radius = outer - (float) b * ringThickness;
        const float inner = radius - ringThickness * 0.82f;
        const float phase = tuner.getStrobePhase (b);
        const int segments = 12 + b * 6;

        for (int s = 0; s < segments; ++s)
        {
            // Every other segment is lit; rotating the whole pattern by the
            // demodulated phase is what produces the apparent drift.
            if (s % 2 != 0) continue;

            const float a0 = juce::MathConstants<float>::twoPi * ((float) s / (float) segments + phase);
            const float a1 = a0 + juce::MathConstants<float>::twoPi / (float) segments;

            juce::Path seg;
            seg.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, a0, a1, true);
            seg.addCentredArc (centre.x, centre.y, inner, inner, 0.0f, a1, a0, false);
            seg.closeSubPath();

            g.setColour (locked ? theme::accent.withAlpha (0.55f + 0.1f * (float) b)
                                : theme::outline.withAlpha (0.5f));
            g.fillPath (seg);
        }
    }

    const float cents = tuner.getCents();
    const bool inTune = locked && std::abs (cents) < 2.0f;

    g.setColour (inTune ? theme::good : (locked ? theme::text : theme::textDim));
    g.setFont (juce::Font (juce::FontOptions (outer * 0.55f, juce::Font::bold)));
    g.drawText (locked ? juce::String (midiNoteName (tuner.getMidiNote())) : juce::String ("--"),
                bounds, juce::Justification::centred, false);
}

// ============================================================================
//  Tuner panel
// ============================================================================

TunerPanel::TunerPanel (BassAmpProcessor& p)
    : processor (p), strobe (p.getRig().tuner())
{
    addAndMakeVisible (strobe);

    noteLabel.setFont (juce::Font (juce::FontOptions (16.0f, juce::Font::bold)));
    noteLabel.setColour (juce::Label::textColourId, theme::text);
    noteLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (noteLabel);

    centsLabel.setFont (juce::Font (juce::FontOptions (13.0f)));
    centsLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (centsLabel);

    frequencyLabel.setFont (juce::Font (juce::FontOptions (12.0f)));
    frequencyLabel.setColour (juce::Label::textColourId, theme::textDim);
    frequencyLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (frequencyLabel);

    for (int i = 0; i < numTuningPresets(); ++i) tuningSelect.addItem (tuningPreset (i).name, i + 1);
    tuningSelect.setSelectedItemIndex (1, juce::dontSendNotification);   // 5-string standard
    tuningSelect.onChange = [this] { rebuildStringButtons(); };
    addAndMakeVisible (tuningSelect);

    referenceA.setSliderStyle (juce::Slider::LinearHorizontal);
    referenceA.setRange (415.0, 466.0, 0.5);
    referenceA.setValue (440.0, juce::dontSendNotification);
    referenceA.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    referenceA.textFromValueFunction = [] (double v) { return "A = " + juce::String (v, 1) + " Hz"; };
    referenceA.onValueChange = [this] { processor.getRig().tuner().setReferenceA ((float) referenceA.getValue()); };
    addAndMakeVisible (referenceA);

    referenceLabel.setText ("Reference", juce::dontSendNotification);
    referenceLabel.setFont (juce::Font (juce::FontOptions (11.0f)));
    referenceLabel.setColour (juce::Label::textColourId, theme::textDim);
    addAndMakeVisible (referenceLabel);

    autoTarget.setToggleState (true, juce::dontSendNotification);
    autoTarget.onClick = [this]
    {
        processor.getRig().tuner().setAutoTarget (autoTarget.getToggleState());
    };
    addAndMakeVisible (autoTarget);

    muteWhileTuning.onClick = [this]
    {
        auto s = processor.getRig().getSettings();
        s.muteWhileTuning = muteWhileTuning.getToggleState();
        processor.getRig().setSettings (s);
    };
    addAndMakeVisible (muteWhileTuning);

    hint.setFont (juce::Font (juce::FontOptions (11.5f)));
    hint.setColour (juce::Label::textColourId, theme::textDim);
    hint.setJustificationType (juce::Justification::topLeft);
    hint.setText ("Play one open string and let it ring. The rings stand still when you are in tune; "
                  "drifting clockwise means sharp, anticlockwise means flat. Lock onto a specific "
                  "string below when you are setting intonation.",
                  juce::dontSendNotification);
    addAndMakeVisible (hint);

    rebuildStringButtons();
    startTimerHz (20);
}

void TunerPanel::visibilityChanged()
{
    // The tuner only runs while its page is open: no reason to pay for pitch
    // detection when nobody is looking at it.
    auto s = processor.getRig().getSettings();
    s.tunerActive = isVisible();
    processor.getRig().setSettings (s);
}

void TunerPanel::rebuildStringButtons()
{
    stringButtons.clear();
    const auto& preset = tuningPreset (tuningSelect.getSelectedItemIndex());

    for (int i = 0; i < preset.numStrings; ++i)
    {
        const int midi = (int) preset.midiNotes[i];
        auto* button = stringButtons.add (new juce::TextButton (
            juce::String (preset.stringNames[i]) + " " + juce::String (midiNoteName (midi))));
        button->onClick = [this, midi]
        {
            autoTarget.setToggleState (false, juce::sendNotification);
            processor.getRig().tuner().setAutoTarget (false);
            processor.getRig().tuner().setTargetMidiNote (midi);
        };
        addAndMakeVisible (button);
    }
    resized();
}

void TunerPanel::timerCallback()
{
    auto& tuner = processor.getRig().tuner();

    if (tuner.hasSignal())
    {
        const float cents = tuner.getCents();
        noteLabel.setText (juce::String (midiNoteName (tuner.getMidiNote())), juce::dontSendNotification);
        centsLabel.setText ((cents >= 0.0f ? "+" : "") + juce::String (cents, 1) + " cents",
                            juce::dontSendNotification);
        centsLabel.setColour (juce::Label::textColourId,
                              std::abs (cents) < 2.0f ? theme::good
                                                      : (std::abs (cents) < 10.0f ? theme::accent : theme::warn));
        frequencyLabel.setText (juce::String (tuner.getFrequency(), 2) + " Hz", juce::dontSendNotification);
    }
    else
    {
        noteLabel.setText ("--", juce::dontSendNotification);
        centsLabel.setText ("play a note", juce::dontSendNotification);
        centsLabel.setColour (juce::Label::textColourId, theme::textDim);
        frequencyLabel.setText ({}, juce::dontSendNotification);
    }
}

void TunerPanel::paint (juce::Graphics& g)
{
    g.setColour (theme::panel);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);
}

void TunerPanel::resized()
{
    auto bounds = getLocalBounds().reduced (14);
    auto left = bounds.removeFromLeft (juce::jmin (300, bounds.getWidth() / 2));

    strobe.setBounds (left.removeFromTop (juce::jmin (260, left.getHeight() - 70)));
    noteLabel.setBounds (left.removeFromTop (22));
    centsLabel.setBounds (left.removeFromTop (20));
    frequencyLabel.setBounds (left.removeFromTop (18));

    bounds.removeFromLeft (18);

    tuningSelect.setBounds (bounds.removeFromTop (28));
    bounds.removeFromTop (10);

    auto strings = bounds.removeFromTop (34);
    const int count = juce::jmax (1, stringButtons.size());
    const int width = juce::jmin (78, strings.getWidth() / count);
    for (auto* b : stringButtons) b->setBounds (strings.removeFromLeft (width).reduced (2, 0));

    bounds.removeFromTop (12);
    autoTarget.setBounds (bounds.removeFromTop (26));
    muteWhileTuning.setBounds (bounds.removeFromTop (26));
    bounds.removeFromTop (10);

    auto refRow = bounds.removeFromTop (26);
    referenceLabel.setBounds (refRow.removeFromLeft (70));
    referenceA.setBounds (refRow);

    bounds.removeFromTop (12);
    hint.setBounds (bounds.removeFromTop (70));
}

// ============================================================================
//  Metronome panel
// ============================================================================

MetronomePanel::MetronomePanel (BassAmpProcessor& p) : processor (p)
{
    auto& metro = processor.getRig().metronome();

    startStop.onClick = [this]
    {
        auto& m = processor.getRig().metronome();
        m.setRunning (! m.isRunning());
        startStop.setButtonText (m.isRunning() ? "Stop" : "Start");
    };
    addAndMakeVisible (startStop);

    tapButton.onClick = [this]
    {
        processor.getRig().metronome().tap (juce::Time::getMillisecondCounterHiRes() * 0.001);
        tempo.setValue (processor.getRig().metronome().getTempo(), juce::dontSendNotification);
    };
    addAndMakeVisible (tapButton);

    tempo.setSliderStyle (juce::Slider::LinearHorizontal);
    tempo.setRange (20.0, 300.0, 0.5);
    tempo.setValue (metro.getTempo(), juce::dontSendNotification);
    tempo.setTextBoxStyle (juce::Slider::TextBoxRight, false, 70, 22);
    tempo.textFromValueFunction = [] (double v) { return juce::String (v, 1) + " BPM"; };
    tempo.onValueChange = [this] { processor.getRig().metronome().setTempo (tempo.getValue()); };
    addAndMakeVisible (tempo);

    tempoLabel.setText ("Tempo", juce::dontSendNotification);

    for (int i = 1; i <= 16; ++i) numerator.addItem (juce::String (i), i);
    numerator.setSelectedId (4, juce::dontSendNotification);
    for (int i = 0; i < 5; ++i) denominator.addItem (juce::String (1 << i), i + 1);
    denominator.setSelectedId (3, juce::dontSendNotification);   // 4

    auto applySignature = [this]
    {
        static const int denominators[] = { 1, 2, 4, 8, 16 };
        processor.getRig().metronome().setTimeSignature (
            numerator.getSelectedId(),
            denominators[juce::jlimit (0, 4, denominator.getSelectedId() - 1)]);
        rebuildAccents();
    };
    numerator.onChange = applySignature;
    denominator.onChange = applySignature;
    addAndMakeVisible (numerator);
    addAndMakeVisible (denominator);
    signatureLabel.setText ("Time signature", juce::dontSendNotification);

    for (int i = 0; i < (int) Subdivision::NumSubdivisions; ++i)
        subdivision.addItem (subdivisionName ((Subdivision) i), i + 1);
    subdivision.setSelectedId (1, juce::dontSendNotification);
    subdivision.onChange = [this]
    {
        processor.getRig().metronome().setSubdivision ((Subdivision) (subdivision.getSelectedId() - 1));
    };
    addAndMakeVisible (subdivision);
    subdivisionLabel.setText ("Click on", juce::dontSendNotification);

    for (int i = 0; i <= 4; ++i) countIn.addItem (i == 0 ? "Off" : juce::String (i) + " bar", i + 1);
    countIn.setSelectedId (1, juce::dontSendNotification);
    countIn.onChange = [this] { processor.getRig().metronome().setCountInBars (countIn.getSelectedId() - 1); };
    addAndMakeVisible (countIn);
    countInLabel.setText ("Count in", juce::dontSendNotification);

    swing.setSliderStyle (juce::Slider::LinearHorizontal);
    swing.setRange (0.0, 0.75, 0.01);
    swing.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 22);
    swing.textFromValueFunction = [] (double v) { return juce::String ((int) (v * 100.0)) + "%"; };
    swing.onValueChange = [this] { processor.getRig().metronome().setSwing ((float) swing.getValue()); };
    addAndMakeVisible (swing);
    swingLabel.setText ("Swing", juce::dontSendNotification);

    level.setSliderStyle (juce::Slider::LinearHorizontal);
    level.setRange (0.0, 1.5, 0.01);
    level.setValue (0.6, juce::dontSendNotification);
    level.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 22);
    level.onValueChange = [this] { processor.getRig().metronome().setLevel ((float) level.getValue()); };
    addAndMakeVisible (level);
    levelLabel.setText ("Click level", juce::dontSendNotification);

    for (auto* l : { &tempoLabel, &swingLabel, &levelLabel, &signatureLabel, &subdivisionLabel, &countInLabel })
    {
        l->setFont (juce::Font (juce::FontOptions (11.5f)));
        l->setColour (juce::Label::textColourId, theme::textDim);
        addAndMakeVisible (*l);
    }

    rebuildAccents();
    startTimerHz (25);
}

void MetronomePanel::rebuildAccents()
{
    const int beats = processor.getRig().metronome().getBeatsPerBar();
    if (beats == lastBeatsPerBar) return;
    lastBeatsPerBar = beats;

    accentButtons.clear();
    for (int i = 0; i < beats; ++i)
    {
        auto* b = accentButtons.add (new juce::TextButton (juce::String (i + 1)));
        b->setClickingTogglesState (false);
        b->onClick = [this, i]
        {
            auto& m = processor.getRig().metronome();
            // Cycles accent -> normal -> silent, which is how you build a clave
            // or an off-beat-only click.
            const auto current = m.getAccent (i);
            m.setAccent (i, current == AccentLevel::Accent ? AccentLevel::Normal
                           : current == AccentLevel::Normal ? AccentLevel::Silent : AccentLevel::Accent);
        };
        addAndMakeVisible (b);
    }
    resized();
}

void MetronomePanel::timerCallback()
{
    auto& metro = processor.getRig().metronome();
    rebuildAccents();

    const int beat = metro.getCurrentBeat();
    const bool running = metro.isRunning();

    for (int i = 0; i < accentButtons.size(); ++i)
    {
        const auto accent = metro.getAccent (i);
        const bool active = running && i == beat;
        auto colour = accent == AccentLevel::Accent ? theme::accent
                    : accent == AccentLevel::Normal ? theme::panelLight : theme::panel;
        accentButtons[i]->setColour (juce::TextButton::buttonColourId,
                                     active ? colour.brighter (0.6f) : colour);
        accentButtons[i]->repaint();
    }

    if (metro.isCountingIn()) startStop.setButtonText ("Counting in");
    else startStop.setButtonText (running ? "Stop" : "Start");
}

void MetronomePanel::paint (juce::Graphics& g)
{
    g.setColour (theme::panel);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);

    g.setColour (theme::textDim);
    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    g.drawText ("Click a beat to cycle it: accented, normal, silent.",
                getLocalBounds().reduced (16).removeFromBottom (20), juce::Justification::bottomLeft, false);
}

void MetronomePanel::resized()
{
    auto bounds = getLocalBounds().reduced (14);

    auto top = bounds.removeFromTop (34);
    startStop.setBounds (top.removeFromLeft (110));
    top.removeFromLeft (8);
    tapButton.setBounds (top.removeFromLeft (70));
    bounds.removeFromTop (12);

    auto row = [&bounds] (int height) { auto r = bounds.removeFromTop (height); bounds.removeFromTop (8); return r; };

    auto tempoRow = row (26);
    tempoLabel.setBounds (tempoRow.removeFromLeft (90));
    tempo.setBounds (tempoRow.removeFromLeft (juce::jmin (360, tempoRow.getWidth())));

    auto sigRow = row (26);
    signatureLabel.setBounds (sigRow.removeFromLeft (90));
    numerator.setBounds (sigRow.removeFromLeft (66));
    sigRow.removeFromLeft (6);
    denominator.setBounds (sigRow.removeFromLeft (66));
    sigRow.removeFromLeft (20);
    subdivisionLabel.setBounds (sigRow.removeFromLeft (70));
    subdivision.setBounds (sigRow.removeFromLeft (110));

    auto swingRow = row (26);
    swingLabel.setBounds (swingRow.removeFromLeft (90));
    swing.setBounds (swingRow.removeFromLeft (juce::jmin (260, swingRow.getWidth())));
    swingRow.removeFromLeft (20);
    countInLabel.setBounds (swingRow.removeFromLeft (70));
    countIn.setBounds (swingRow.removeFromLeft (90));

    auto levelRow = row (26);
    levelLabel.setBounds (levelRow.removeFromLeft (90));
    level.setBounds (levelRow.removeFromLeft (juce::jmin (260, levelRow.getWidth())));

    bounds.removeFromTop (10);
    auto accents = bounds.removeFromTop (40);
    const int count = juce::jmax (1, accentButtons.size());
    const int width = juce::jmin (46, accents.getWidth() / count);
    for (auto* b : accentButtons) b->setBounds (accents.removeFromLeft (width).reduced (2));
}

// ============================================================================
//  Backing track panel
// ============================================================================

TrackPanel::TrackPanel (BassAmpProcessor& p) : processor (p)
{
    loadButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Choose a backing track", juce::File(),
                                                       "*.wav;*.aiff;*.aif;*.flac;*.mp3;*.ogg");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& fc)
        {
            const auto file = fc.getResult();
            if (file == juce::File()) return;
            const auto error = processor.getTrackPlayer().loadFile (file);
            if (error.isNotEmpty())
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Could not load track", error);
            else
                fileLabel.setText (processor.getTrackPlayer().getFileName(), juce::dontSendNotification);
        });
    };
    addAndMakeVisible (loadButton);

    playButton.onClick = [this] { processor.getTrackPlayer().togglePlay(); };
    stopButton.onClick = [this]
    {
        processor.getTrackPlayer().stop();
        processor.getTrackPlayer().setPosition (0.0);
    };
    addAndMakeVisible (playButton);
    addAndMakeVisible (stopButton);

    loopButton.setToggleState (true, juce::dontSendNotification);
    loopButton.onClick = [this] { processor.getTrackPlayer().setLooping (loopButton.getToggleState()); };
    addAndMakeVisible (loopButton);

    position.setSliderStyle (juce::Slider::LinearHorizontal);
    position.setRange (0.0, 1.0, 0.0001);
    position.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    position.onDragStart = [this] { dragging = true; };
    position.onDragEnd = [this]
    {
        dragging = false;
        auto& player = processor.getTrackPlayer();
        player.setPosition (position.getValue() * juce::jmax (0.001, player.getLength()));
    };
    addAndMakeVisible (position);

    level.setSliderStyle (juce::Slider::LinearHorizontal);
    level.setRange (-60.0, 6.0, 0.1);
    level.setValue (-6.0, juce::dontSendNotification);
    level.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 22);
    level.textFromValueFunction = [] (double v) { return juce::String (v, 1) + " dB"; };
    level.onValueChange = [this] { processor.getTrackPlayer().setLevelDb ((float) level.getValue()); };
    addAndMakeVisible (level);

    bassCut.setSliderStyle (juce::Slider::LinearHorizontal);
    bassCut.setRange (0.0, 1.0, 0.01);
    bassCut.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 22);
    bassCut.textFromValueFunction = [] (double v) { return juce::String ((int) (v * 100.0)) + "%"; };
    bassCut.onValueChange = [this] { processor.getTrackPlayer().setBassCut ((float) bassCut.getValue()); };
    addAndMakeVisible (bassCut);

    for (auto* l : { &levelLabel, &bassCutLabel, &fileLabel, &timeLabel })
    {
        l->setFont (juce::Font (juce::FontOptions (11.5f)));
        l->setColour (juce::Label::textColourId, theme::textDim);
        addAndMakeVisible (*l);
    }
    levelLabel.setText ("Track level", juce::dontSendNotification);
    bassCutLabel.setText ("Cut recorded bass", juce::dontSendNotification);
    fileLabel.setText ("No track loaded", juce::dontSendNotification);

    hint.setFont (juce::Font (juce::FontOptions (11.5f)));
    hint.setColour (juce::Label::textColourId, theme::textDim);
    hint.setJustificationType (juce::Justification::topLeft);
    hint.setText ("The track is mixed in after the rig, so none of your amp or pedal settings touch it. "
                  "Cut recorded bass removes centre-panned low frequencies, which pulls the original "
                  "bass part down without gutting the rest of the mix.",
                  juce::dontSendNotification);
    addAndMakeVisible (hint);

    startTimerHz (12);
}

void TrackPanel::timerCallback()
{
    auto& player = processor.getTrackPlayer();
    playButton.setButtonText (player.isPlaying() ? "Pause" : "Play");

    const double length = player.getLength();
    if (! dragging && length > 0.0)
        position.setValue (player.getPosition() / length, juce::dontSendNotification);

    auto format = [] (double seconds)
    {
        const int total = (int) seconds;
        return juce::String (total / 60) + ":" + juce::String (total % 60).paddedLeft ('0', 2);
    };
    timeLabel.setText (length > 0.0 ? format (player.getPosition()) + " / " + format (length)
                                    : juce::String(), juce::dontSendNotification);
}

void TrackPanel::paint (juce::Graphics& g)
{
    g.setColour (theme::panel);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);
}

void TrackPanel::resized()
{
    auto bounds = getLocalBounds().reduced (14);

    auto top = bounds.removeFromTop (30);
    loadButton.setBounds (top.removeFromLeft (120));
    top.removeFromLeft (8);
    playButton.setBounds (top.removeFromLeft (80));
    top.removeFromLeft (6);
    stopButton.setBounds (top.removeFromLeft (70));
    top.removeFromLeft (10);
    loopButton.setBounds (top.removeFromLeft (80));
    bounds.removeFromTop (10);

    fileLabel.setBounds (bounds.removeFromTop (18));
    bounds.removeFromTop (6);

    position.setBounds (bounds.removeFromTop (24));
    timeLabel.setBounds (bounds.removeFromTop (18));
    bounds.removeFromTop (10);

    auto levelRow = bounds.removeFromTop (26);
    levelLabel.setBounds (levelRow.removeFromLeft (120));
    level.setBounds (levelRow.removeFromLeft (juce::jmin (300, levelRow.getWidth())));
    bounds.removeFromTop (8);

    auto cutRow = bounds.removeFromTop (26);
    bassCutLabel.setBounds (cutRow.removeFromLeft (120));
    bassCut.setBounds (cutRow.removeFromLeft (juce::jmin (300, cutRow.getWidth())));

    bounds.removeFromTop (14);
    hint.setBounds (bounds.removeFromTop (70));
}

// ============================================================================
//  Settings panel
// ============================================================================

SettingsPanel::SettingsPanel (BassAmpProcessor& p) : processor (p)
{
   #if JucePlugin_Build_Standalone
    audioSettings.onClick = []
    {
        if (auto* holder = juce::StandalonePluginHolder::getInstance())
            holder->showAudioSettingsDialog();
    };
    addAndMakeVisible (audioSettings);
   #endif

    oversampling.addItem ("Off - lowest CPU", 1);
    oversampling.addItem ("2x - live (recommended)", 2);
    oversampling.addItem ("4x - studio", 3);
    oversampling.setSelectedId (2, juce::dontSendNotification);
    oversampling.onChange = [this]
    {
        static const int factors[] = { 1, 2, 4 };
        auto s = processor.getRig().getSettings();
        s.oversampling = factors[juce::jlimit (0, 2, oversampling.getSelectedId() - 1)];
        processor.getRig().setSettings (s);
    };
    addAndMakeVisible (oversampling);
    oversamplingLabel.setText ("Distortion oversampling", juce::dontSendNotification);

    for (int cc = 1; cc <= 31; ++cc) expressionCc.addItem ("CC " + juce::String (cc), cc);
    expressionCc.setSelectedId (11, juce::dontSendNotification);
    expressionCc.onChange = [this] { processor.setExpressionCc (expressionCc.getSelectedId()); };
    addAndMakeVisible (expressionCc);
    expressionLabel.setText ("Expression pedal MIDI CC", juce::dontSendNotification);

    expressionPedal.setSliderStyle (juce::Slider::LinearHorizontal);
    expressionPedal.setRange (0.0, 1.0, 0.001);
    expressionPedal.setValue (0.5, juce::dontSendNotification);
    expressionPedal.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    expressionPedal.onValueChange = [this] { processor.setExpression ((float) expressionPedal.getValue()); };
    addAndMakeVisible (expressionPedal);
    expressionPedalLabel.setText ("Pedal position (wah / filter)", juce::dontSendNotification);

    for (auto* l : { &oversamplingLabel, &expressionLabel, &expressionPedalLabel, &statusLabel })
    {
        l->setFont (juce::Font (juce::FontOptions (11.5f)));
        l->setColour (juce::Label::textColourId, theme::textDim);
        addAndMakeVisible (*l);
    }

    latencyHelp.setFont (juce::Font (juce::FontOptions (11.5f)));
    latencyHelp.setColour (juce::Label::textColourId, theme::textDim);
    latencyHelp.setJustificationType (juce::Justification::topLeft);
    latencyHelp.setText (
        "The rig itself adds no latency: the cabinet convolution has a direct path for its first "
        "128 taps, and the oversampling filters are IIR rather than linear phase. Everything you "
        "feel is your interface's buffer plus its converters.\n\n"
        "On Windows, use the ASIO driver your interface ships with and set the buffer to 64 or 128 "
        "samples. WASAPI in exclusive mode is the fallback if there is no ASIO driver. Avoid "
        "MME and DirectSound - they will cost you 20 ms or more before anything here matters.",
        juce::dontSendNotification);
    addAndMakeVisible (latencyHelp);

    startTimerHz (4);
}

void SettingsPanel::timerCallback()
{
    const double sr = processor.getCurrentSampleRate();
    const int block = processor.getCurrentBlockSize();
    const double bufferMs = sr > 0.0 ? (double) block / sr * 1000.0 : 0.0;

    statusLabel.setText (juce::String (sr / 1000.0, 1) + " kHz, " + juce::String (block) + " samples ("
                         + juce::String (bufferMs, 2) + " ms per buffer)   |   plugin latency: 0 samples"
                         + "   |   CPU: " + juce::String (processor.getCpuLoad() * 100.0f, 1) + "%",
                         juce::dontSendNotification);
}

void SettingsPanel::paint (juce::Graphics& g)
{
    g.setColour (theme::panel);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);
}

void SettingsPanel::resized()
{
    auto bounds = getLocalBounds().reduced (14);

   #if JucePlugin_Build_Standalone
    audioSettings.setBounds (bounds.removeFromTop (32).removeFromLeft (220));
    bounds.removeFromTop (12);
   #endif

    statusLabel.setBounds (bounds.removeFromTop (20));
    bounds.removeFromTop (12);

    auto row = bounds.removeFromTop (26);
    oversamplingLabel.setBounds (row.removeFromLeft (200));
    oversampling.setBounds (row.removeFromLeft (200));
    bounds.removeFromTop (8);

    row = bounds.removeFromTop (26);
    expressionLabel.setBounds (row.removeFromLeft (200));
    expressionCc.setBounds (row.removeFromLeft (120));
    bounds.removeFromTop (8);

    row = bounds.removeFromTop (26);
    expressionPedalLabel.setBounds (row.removeFromLeft (200));
    expressionPedal.setBounds (row.removeFromLeft (juce::jmin (300, row.getWidth())));

    bounds.removeFromTop (16);
    latencyHelp.setBounds (bounds);
}
