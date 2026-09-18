#include "Panels.h"

using namespace bassamp;

// ============================================================================
//  Amp panel
// ============================================================================

AmpPanel::AmpPanel (BassAmpProcessor& p)
    : processor (p),
      toneCurve ([&p] (float freq) { return p.getRig().amp().toneMagnitudeDbAt (freq); })
{
    auto& state = processor.getState();

    for (int i = 0; i < (int) AmpType::NumTypes; ++i) ampSelect.addItem (ampName ((AmpType) i), i + 1);
    addAndMakeVisible (ampSelect);
    ampAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, "amp_type", ampSelect);

    auto makeKnob = [this, &state] (std::unique_ptr<Knob>& k, const char* id, const char* label)
    {
        k = std::make_unique<Knob> (state, id, label);
        addAndMakeVisible (*k);
    };

    makeKnob (gain,     "amp_gain",     "Gain");
    makeKnob (bass,     "amp_bass",     "Bass");
    makeKnob (mid,      "amp_mid",      "Mid");
    makeKnob (treble,   "amp_treble",   "Treble");
    makeKnob (presence, "amp_presence", "Presence");
    makeKnob (master,   "amp_master",   "Master");
    makeKnob (output,   "amp_output",   "Output");
    makeKnob (sag,      "amp_sag",      "Sag");
    makeKnob (blend,    "amp_blend",    "Blend");
    makeKnob (sub,      "amp_sub",      "Sub");

    addAndMakeVisible (midFreq);
    midFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, "amp_midfreq", midFreq);
    midFreqLabel.setText ("Mid freq", juce::dontSendNotification);
    midFreqLabel.setFont (juce::Font (juce::FontOptions (11.0f)));
    midFreqLabel.setColour (juce::Label::textColourId, theme::textDim);
    midFreqLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (midFreqLabel);

    for (auto* b : { &bright, &ultraLo, &ultraHi, &shape }) addAndMakeVisible (*b);
    brightAtt  = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "amp_bright",  bright);
    ultraLoAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "amp_ultralo", ultraLo);
    ultraHiAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "amp_ultrahi", ultraHi);
    shapeAtt   = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "amp_shape",   shape);

    addAndMakeVisible (toneCurve);

    description.setFont (juce::Font (juce::FontOptions (12.5f)));
    description.setColour (juce::Label::textColourId, theme::textDim);
    description.setJustificationType (juce::Justification::topLeft);
    addAndMakeVisible (description);

    sagReadout.setFont (juce::Font (juce::FontOptions (11.0f)));
    sagReadout.setColour (juce::Label::textColourId, theme::textDim);
    sagReadout.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (sagReadout);

    refreshForAmp();
    startTimerHz (10);
}

void AmpPanel::refreshForAmp()
{
    const AmpType type = processor.getRig().amp().getSettings().type;
    lastAmpType = (int) type;

    description.setText (ampDescription (type), juce::dontSendNotification);

    // Show only the controls the amp actually has. A Bassman does not have an
    // Ultra Lo switch, and pretending otherwise is how modellers lose trust.
    bright.setVisible  (ampHasBright (type));
    ultraLo.setVisible (ampHasUltra (type));
    ultraHi.setVisible (ampHasUltra (type));
    shape.setVisible   (ampHasShape (type));
    sub->setVisible    (ampHasSub (type));
    blend->setVisible  (ampHasBlend (type));
    presence->setVisible (ampHasPresence (type));
    midFreq.setVisible (ampHasMidFreq (type));
    midFreqLabel.setVisible (ampHasMidFreq (type));

    if (ampHasMidFreq (type))
    {
        // Retitle rather than repopulate: clearing a combo box out from under a
        // live parameter attachment loses the selection.
        for (int i = 0; i < 5; ++i)
            midFreq.changeItemText (i + 1, juce::String ((int) ampMidFreqValue (type, i)) + " Hz");
        midFreq.setText (midFreq.getItemText (juce::jmax (0, midFreq.getSelectedItemIndex())),
                         juce::dontSendNotification);
    }

    resized();
}

void AmpPanel::timerCallback()
{
    if ((int) processor.getRig().amp().getSettings().type != lastAmpType) refreshForAmp();
    sagReadout.setText ("Rail sag " + juce::String (processor.getRig().amp().getSagDb(), 1) + " dB",
                        juce::dontSendNotification);
}

void AmpPanel::paint (juce::Graphics& g)
{
    g.setColour (theme::panel);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);
}

void AmpPanel::resized()
{
    auto bounds = getLocalBounds().reduced (12);

    auto top = bounds.removeFromTop (30);
    ampSelect.setBounds (top.removeFromLeft (280));
    sagReadout.setBounds (top.removeFromRight (140));
    bounds.removeFromTop (8);

    description.setBounds (bounds.removeFromTop (54));
    bounds.removeFromTop (6);

    auto knobRow = bounds.removeFromTop (92);
    const int knobWidth = 84;
    auto place = [&knobRow, knobWidth] (juce::Component& c)
    {
        if (! c.isVisible()) return;
        c.setBounds (knobRow.removeFromLeft (knobWidth));
    };

    place (*gain);
    place (*bass);
    if (midFreq.isVisible())
    {
        auto col = knobRow.removeFromLeft (knobWidth).reduced (4, 0);
        midFreqLabel.setBounds (col.removeFromTop (16));
        midFreq.setBounds (col.removeFromTop (26));
    }
    place (*mid);
    place (*treble);
    place (*presence);
    place (*master);
    place (*output);
    place (*sag);
    place (*blend);
    place (*sub);

    bounds.removeFromTop (8);
    auto switchRow = bounds.removeFromTop (28);
    for (auto* b : { &bright, &ultraLo, &ultraHi, &shape })
        if (b->isVisible()) b->setBounds (switchRow.removeFromLeft (110).reduced (2, 0));

    bounds.removeFromTop (10);
    toneCurve.setBounds (bounds);
}

// ============================================================================
//  Cabinet panel
// ============================================================================

CabPanel::CabPanel (BassAmpProcessor& p)
    : processor (p),
      cabCurve ([&p] (float freq) { return p.getRig().cabinet().magnitudeDbAt (freq); })
{
    auto& state = processor.getState();

    for (int i = 0; i < (int) CabType::NumTypes; ++i) cabSelect.addItem (cabName ((CabType) i), i + 1);
    for (int i = 0; i < (int) MicType::NumTypes; ++i) micSelect.addItem (micName ((MicType) i), i + 1);
    addAndMakeVisible (cabSelect);
    addAndMakeVisible (micSelect);
    cabAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, "cab_type", cabSelect);
    micAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, "cab_mic", micSelect);

    auto makeKnob = [this, &state] (std::unique_ptr<Knob>& k, const char* id, const char* label)
    {
        k = std::make_unique<Knob> (state, id, label);
        addAndMakeVisible (*k);
    };
    makeKnob (position, "cab_position", "Position");
    makeKnob (distance, "cab_distance", "Distance");
    makeKnob (room,     "cab_room",     "Room");
    makeKnob (lowCut,   "cab_lowcut",   "Low Cut");
    makeKnob (highCut,  "cab_highcut",  "High Cut");

    addAndMakeVisible (horn);
    hornAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "cab_horn", horn);

    addAndMakeVisible (loadIr);
    addAndMakeVisible (clearIr);
    loadIr.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Load an impulse response", juce::File(), "*.wav");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& fc)
        {
            const auto file = fc.getResult();
            if (file == juce::File()) return;
            const auto error = processor.loadUserIr (file);
            if (error.isNotEmpty())
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Could not load IR", error);
            else
                irName.setText ("IR: " + processor.getUserIrName(), juce::dontSendNotification);
        });
    };
    clearIr.onClick = [this]
    {
        if (auto* param = processor.getState().getParameter ("cab_type"))
            param->setValueNotifyingHost (param->convertTo0to1 ((float) (int) CabType::C4x10));
    };

    irName.setFont (juce::Font (juce::FontOptions (11.5f)));
    irName.setColour (juce::Label::textColourId, theme::textDim);
    addAndMakeVisible (irName);

    description.setFont (juce::Font (juce::FontOptions (12.5f)));
    description.setColour (juce::Label::textColourId, theme::textDim);
    description.setJustificationType (juce::Justification::topLeft);
    addAndMakeVisible (description);

    addAndMakeVisible (cabCurve);
    startTimerHz (6);
}

void CabPanel::timerCallback()
{
    const int type = (int) processor.getRig().getCabinetSettings().cab;
    if (type == lastCabType) return;
    lastCabType = type;

    description.setText (cabDescription ((CabType) type), juce::dontSendNotification);
    horn.setEnabled (cabHasHorn ((CabType) type));
    const bool modelled = type != (int) CabType::Bypass && type != (int) CabType::UserIr;
    position->setEnabled (modelled);
    distance->setEnabled (modelled);
    room->setEnabled (modelled);
    micSelect.setEnabled (modelled);
}

void CabPanel::paint (juce::Graphics& g)
{
    g.setColour (theme::panel);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);
}

void CabPanel::resized()
{
    auto bounds = getLocalBounds().reduced (12);

    auto top = bounds.removeFromTop (30);
    cabSelect.setBounds (top.removeFromLeft (200));
    top.removeFromLeft (8);
    micSelect.setBounds (top.removeFromLeft (180));
    top.removeFromLeft (8);
    horn.setBounds (top.removeFromLeft (90));
    bounds.removeFromTop (8);

    description.setBounds (bounds.removeFromTop (48));
    bounds.removeFromTop (6);

    auto knobRow = bounds.removeFromTop (92);
    for (auto* k : { position.get(), distance.get(), room.get(), lowCut.get(), highCut.get() })
        k->setBounds (knobRow.removeFromLeft (84));

    auto irRow = knobRow.removeFromLeft (200).reduced (6, 12);
    loadIr.setBounds (irRow.removeFromTop (26));
    irRow.removeFromTop (4);
    clearIr.setBounds (irRow.removeFromTop (26));

    bounds.removeFromTop (4);
    irName.setBounds (bounds.removeFromTop (18));
    bounds.removeFromTop (6);
    cabCurve.setBounds (bounds);
}

// ============================================================================
//  Pedalboard panel
// ============================================================================

PedalboardPanel::Row::Row (PedalboardPanel& owner, int typeIndex)
    : panel (owner), type (typeIndex)
{
    power.setButtonText (pedalTypeName ((PedalType) typeIndex));
    addAndMakeVisible (power);
    powerAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        panel.processor.getState(), ParameterBridge::pedalEnabledId (typeIndex), power);

    up.setConnectedEdges (juce::Button::ConnectedOnBottom);
    down.setConnectedEdges (juce::Button::ConnectedOnTop);
    addAndMakeVisible (up);
    addAndMakeVisible (down);

    up.onClick = [this]
    {
        auto& board = panel.processor.getRig().pedalboard();
        const int index = board.chainIndexOfType ((PedalType) type);
        if (index > 0) board.moveTo ((PedalType) type, index - 1);
        panel.rebuildList();
    };
    down.onClick = [this]
    {
        auto& board = panel.processor.getRig().pedalboard();
        const int index = board.chainIndexOfType ((PedalType) type);
        if (index < Pedalboard::kNumSlots - 1) board.moveTo ((PedalType) type, index + 1);
        panel.rebuildList();
    };
}

void PedalboardPanel::Row::mouseDown (const juce::MouseEvent&) { panel.selectPedal (type); }

void PedalboardPanel::Row::paint (juce::Graphics& g)
{
    if (selected)
    {
        g.setColour (theme::accentDim.withAlpha (0.28f));
        g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 4.0f);
    }
    const auto place = panel.processor.getRig().pedalboard().placementOf ((PedalType) type);
    if (place != PedalPlacement::FrontOfAmp)
    {
        g.setColour (theme::blue);
        g.setFont (juce::Font (juce::FontOptions (9.5f)));
        g.drawText (place == PedalPlacement::AmpLoop ? "LOOP" : "POST",
                    getLocalBounds().removeFromRight (74).removeFromLeft (36),
                    juce::Justification::centred, false);
    }
}

void PedalboardPanel::Row::resized()
{
    auto bounds = getLocalBounds().reduced (2);
    auto arrows = bounds.removeFromRight (22);
    up.setBounds (arrows.removeFromTop (arrows.getHeight() / 2));
    down.setBounds (arrows);
    bounds.removeFromRight (40);
    power.setBounds (bounds);
}

PedalboardPanel::PedalboardPanel (BassAmpProcessor& p) : processor (p)
{
    addAndMakeVisible (listViewport);
    listViewport.setViewedComponent (&listContent, false);
    listViewport.setScrollBarsShown (true, false);

    for (int i = 0; i < Pedalboard::kNumSlots; ++i)
    {
        auto* row = rows.add (new Row (*this, i));
        listContent.addAndMakeVisible (row);
    }

    pedalTitle.setFont (juce::Font (juce::FontOptions (17.0f, juce::Font::bold)));
    pedalTitle.setColour (juce::Label::textColourId, theme::text);
    addAndMakeVisible (pedalTitle);

    pedalDescription.setFont (juce::Font (juce::FontOptions (12.5f)));
    pedalDescription.setColour (juce::Label::textColourId, theme::textDim);
    pedalDescription.setJustificationType (juce::Justification::topLeft);
    addAndMakeVisible (pedalDescription);

    placementLabel.setText ("Position in rig", juce::dontSendNotification);
    placementLabel.setFont (juce::Font (juce::FontOptions (11.0f)));
    placementLabel.setColour (juce::Label::textColourId, theme::textDim);
    addAndMakeVisible (placementLabel);
    addAndMakeVisible (placement);

    addAndMakeVisible (resetPedal);
    resetPedal.onClick = [this]
    {
        auto probe = createPedal ((PedalType) selectedPedal);
        if (probe == nullptr) return;
        for (int q = 0; q < probe->numParams(); ++q)
            if (auto* param = processor.getState().getParameter (ParameterBridge::pedalParamId (selectedPedal, q)))
                param->setValueNotifyingHost (param->convertTo0to1 (probe->param (q).defaultValue));
    };

    selectPedal (selectedPedal);
    rebuildList();
    startTimerHz (4);
}

void PedalboardPanel::timerCallback()
{
    for (auto* row : rows) row->repaint();
}

void PedalboardPanel::rebuildList()
{
    const auto order = processor.getRig().pedalboard().getOrder();
    const int rowHeight = 30;

    listContent.setSize (juce::jmax (100, listViewport.getWidth() - 10), rowHeight * Pedalboard::kNumSlots);

    for (int i = 0; i < Pedalboard::kNumSlots; ++i)
    {
        const int typeIndex = order[(size_t) i];
        for (auto* row : rows)
        {
            if (row->type != typeIndex) continue;
            row->setBounds (0, i * rowHeight, listContent.getWidth(), rowHeight);
            row->selected = (row->type == selectedPedal);
            row->repaint();
        }
    }
}

void PedalboardPanel::selectPedal (int pedalTypeIndex)
{
    selectedPedal = pedalTypeIndex;
    buildControlsFor (pedalTypeIndex);
    for (auto* row : rows) { row->selected = (row->type == pedalTypeIndex); row->repaint(); }
    resized();
}

void PedalboardPanel::buildControlsFor (int pedalTypeIndex)
{
    // Order matters: an attachment holds a reference to its combo box, so the
    // attachments have to go first.
    choiceAttachments.clear();
    choiceBoxes.clear();
    choiceLabels.clear();
    knobs.clear();

    auto probe = createPedal ((PedalType) pedalTypeIndex);
    if (probe == nullptr) return;

    pedalTitle.setText (probe->name(), juce::dontSendNotification);
    pedalDescription.setText (probe->description(), juce::dontSendNotification);

    placementAttachment.reset();
    placement.clear (juce::dontSendNotification);
    for (int i = 0; i < (int) PedalPlacement::NumPlacements; ++i)
        placement.addItem (placementName ((PedalPlacement) i), i + 1);
    placementAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.getState(), ParameterBridge::pedalPlacementId (pedalTypeIndex), placement);

    auto& state = processor.getState();
    for (int q = 0; q < probe->numParams(); ++q)
    {
        const auto& d = probe->param (q);
        const auto id = ParameterBridge::pedalParamId (pedalTypeIndex, q);

        if (d.numChoices > 0)
        {
            auto* label = choiceLabels.add (new juce::Label ({}, d.name));
            label->setFont (juce::Font (juce::FontOptions (11.0f)));
            label->setColour (juce::Label::textColourId, theme::textDim);
            label->setJustificationType (juce::Justification::centred);
            addAndMakeVisible (label);

            auto* box = choiceBoxes.add (new juce::ComboBox());
            for (int c = 0; c < d.numChoices; ++c)
                box->addItem (d.choices != nullptr ? d.choices[c] : juce::String (c + 1), c + 1);
            addAndMakeVisible (box);
            choiceAttachments.add (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (state, id, *box));
        }
        else
        {
            auto* knob = knobs.add (new Knob (state, id, d.name));
            addAndMakeVisible (knob);
        }
    }
}

void PedalboardPanel::paint (juce::Graphics& g)
{
    g.setColour (theme::panel);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);

    g.setColour (theme::panelLight);
    g.fillRoundedRectangle (getLocalBounds().reduced (10).removeFromLeft (232).toFloat(), 5.0f);

    g.setColour (theme::textDim);
    g.setFont (juce::Font (juce::FontOptions (10.5f)));
    g.drawText ("SIGNAL CHAIN  (top = first)", getLocalBounds().reduced (16).removeFromTop (14).removeFromLeft (220),
                juce::Justification::centredLeft, false);
}

void PedalboardPanel::resized()
{
    auto bounds = getLocalBounds().reduced (12);

    auto left = bounds.removeFromLeft (230);
    left.removeFromTop (18);
    listViewport.setBounds (left);
    rebuildList();

    bounds.removeFromLeft (14);

    pedalTitle.setBounds (bounds.removeFromTop (24));
    pedalDescription.setBounds (bounds.removeFromTop (64));
    bounds.removeFromTop (6);

    auto placeRow = bounds.removeFromTop (26);
    placementLabel.setBounds (placeRow.removeFromLeft (100));
    placement.setBounds (placeRow.removeFromLeft (160));
    placeRow.removeFromLeft (10);
    resetPedal.setBounds (placeRow.removeFromLeft (80));

    bounds.removeFromTop (12);

    // Knobs first, then any choice controls underneath.
    auto knobRow = bounds.removeFromTop (92);
    for (auto* k : knobs)
    {
        if (knobRow.getWidth() < 84)
        {
            knobRow = bounds.removeFromTop (92);
            if (knobRow.getHeight() < 60) break;
        }
        k->setBounds (knobRow.removeFromLeft (84));
    }

    bounds.removeFromTop (8);
    auto choiceRow = bounds.removeFromTop (46);
    for (int i = 0; i < choiceBoxes.size(); ++i)
    {
        auto col = choiceRow.removeFromLeft (128).reduced (4, 0);
        choiceLabels[i]->setBounds (col.removeFromTop (16));
        choiceBoxes[i]->setBounds (col.removeFromTop (26));
    }
}
