#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace bassamp;

BassAmpProcessor::BassAmpProcessor()
    : juce::AudioProcessor (BusesProperties()
                                .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "BASSAMP", ParameterBridge::createLayout())
{
    bridge.connect (parameters);

    // The chain layout is structure rather than automation, so it lives in the
    // state tree alongside the parameters.
    parameters.state.setProperty ("chainOrder", juce::var (juce::String()), nullptr);
    parameters.state.setProperty ("presetName", juce::var (juce::String ("Fridge Standard")), nullptr);

    tunerThread = std::make_unique<TunerAnalysisThread> (rig.tuner());
    startTimerHz (30);
}

BassAmpProcessor::~BassAmpProcessor()
{
    stopTimer();
    if (tunerThread != nullptr) tunerThread->stopThread (1000);
}

void BassAmpProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    rig.prepare (sampleRate, samplesPerBlock);
    trackPlayer.prepare (sampleRate, samplesPerBlock);
    monoInput.setSize (1, juce::jmax (1, samplesPerBlock));

    // Nothing in the chain introduces a delay, and the host needs to know that.
    setLatencySamples (0);

    if (tunerThread != nullptr && ! tunerThread->isThreadRunning())
        tunerThread->startThread (juce::Thread::Priority::low);
}

void BassAmpProcessor::releaseResources()
{
    trackPlayer.releaseResources();
}

bool BassAmpProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    const auto& in = layouts.getMainInputChannelSet();
    return in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo()
        || in == juce::AudioChannelSet::disabled();
}

void BassAmpProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const auto startTime = juce::Time::getHighResolutionTicks();

    const int numSamples = buffer.getNumSamples();
    const int numInputs  = getTotalNumInputChannels();
    const int numOutputs = getTotalNumOutputChannels();

    // MIDI expression: a real expression pedal is far better than dragging a
    // slider with a mouse while you are holding a bass.
    for (const auto meta : midi)
    {
        const auto message = meta.getMessage();
        if (message.isController() && message.getControllerNumber() == expressionCc)
            expression.store ((float) message.getControllerValue() / 127.0f);
    }

    bridge.applyToRig (rig);

    // The wah's pedal position is driven by the expression control rather than
    // by its own parameter when a pedal is connected.
    if (auto* wah = rig.pedalboard().pedalOfType (PedalType::Wah))
        if (wah->isEnabled() && wah->getParamValue (4) < 0.5f)   // pedal mode
            wah->setParamValue (0, expression.load() * 100.0f);

    rig.metronome().setTempo (rig.metronome().getTempo());
    rig.pedalboard().setTempo (rig.metronome().getTempo());

    if (monoInput.getNumSamples() < numSamples) monoInput.setSize (1, numSamples, false, false, true);
    float* mono = monoInput.getWritePointer (0);

    if (numInputs <= 0)
    {
        juce::FloatVectorOperations::clear (mono, numSamples);
    }
    else if (numInputs == 1)
    {
        juce::FloatVectorOperations::copy (mono, buffer.getReadPointer (0), numSamples);
    }
    else
    {
        // A bass through an interface is mono. Summing rather than taking the
        // left channel means it works whichever input the player used.
        const float* l = buffer.getReadPointer (0);
        const float* r = buffer.getReadPointer (1);
        for (int n = 0; n < numSamples; ++n) mono[n] = 0.5f * (l[n] + r[n]);
    }

    float* outL = buffer.getWritePointer (0);
    float* outR = numOutputs > 1 ? buffer.getWritePointer (1) : outL;

    rig.process (mono, outL, outR, numSamples);
    trackPlayer.mixInto (outL, outR, numSamples);

    if (numOutputs > 2)
        for (int c = 2; c < numOutputs; ++c) buffer.clear (c, 0, numSamples);

    const double elapsed = juce::Time::highResolutionTicksToSeconds (juce::Time::getHighResolutionTicks() - startTime);
    const double budget = (double) numSamples / currentSampleRate;
    const float load = (float) juce::jlimit (0.0, 1.5, elapsed / juce::jmax (1.0e-9, budget));
    cpuLoad.store (cpuLoad.load() * 0.9f + load * 0.1f);
}

void BassAmpProcessor::timerCallback()
{
    // Cabinet IR design and swap handshaking happen here, off the audio thread.
    rig.serviceMessageThread();
}

juce::AudioProcessorEditor* BassAmpProcessor::createEditor()
{
    return new BassAmpEditor (*this);
}

void BassAmpProcessor::loadPreset (int index)
{
    if (index < 0 || index >= numPresets()) return;
    currentProgram = index;
    const auto& preset = getPreset (index);
    ParameterBridge::applyPresetToParameters (parameters, rig, preset);
    parameters.state.setProperty ("presetName", juce::var (juce::String (preset.name)), nullptr);
    rig.serviceMessageThread();
}

void BassAmpProcessor::setCurrentProgram (int index) { loadPreset (index); }

const juce::String BassAmpProcessor::getProgramName (int index)
{
    if (index < 0 || index >= numPresets()) return {};
    return getPreset (index).name;
}

juce::String BassAmpProcessor::loadUserIr (const juce::File& file)
{
    const auto error = rig.cabinet().loadUserIr (file.getFullPathName().toStdString());
    if (! error.empty()) return juce::String (error);

    userIrName = rig.cabinet().getUserIrName();
    parameters.state.setProperty ("userIrPath", juce::var (file.getFullPathName()), nullptr);

    if (auto* p = parameters.getParameter ("cab_type"))
        p->setValueNotifyingHost (p->convertTo0to1 ((float) (int) CabType::UserIr));

    return {};
}

void BassAmpProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    juce::String order;
    for (int v : rig.pedalboard().getOrder()) order += (order.isEmpty() ? "" : ",") + juce::String (v);
    state.setProperty ("chainOrder", order, nullptr);

    for (int i = 0; i < Pedalboard::kNumSlots; ++i)
        state.setProperty ("place" + juce::String (i),
                           (int) rig.pedalboard().placementOf ((PedalType) i), nullptr);

    const auto ts = rig.metronome().getTimeSignature();
    state.setProperty ("metroBpm",   rig.metronome().getTempo(), nullptr);
    state.setProperty ("metroNum",   ts.numerator, nullptr);
    state.setProperty ("metroDen",   ts.denominator, nullptr);
    state.setProperty ("metroSub",   (int) rig.metronome().getSubdivision(), nullptr);
    state.setProperty ("metroSwing", rig.metronome().getSwing(), nullptr);
    state.setProperty ("tunerRefA",  rig.tuner().getReferenceA(), nullptr);
    state.setProperty ("expressionCc", expressionCc, nullptr);
    state.setProperty ("oversampling", rig.getSettings().oversampling, nullptr);

    if (auto xml = state.createXml()) copyXmlToBinary (*xml, destData);
}

void BassAmpProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (parameters.state.getType())) return;

    auto state = juce::ValueTree::fromXml (*xml);
    parameters.replaceState (state);

    const juce::String order = state.getProperty ("chainOrder", juce::String()).toString();
    if (order.isNotEmpty())
    {
        juce::StringArray parts;
        parts.addTokens (order, ",", "");
        std::array<int, Pedalboard::kNumSlots> newOrder { };
        for (int i = 0; i < Pedalboard::kNumSlots; ++i)
            newOrder[(size_t) i] = (i < parts.size()) ? parts[i].getIntValue() : i;
        rig.pedalboard().setOrder (newOrder);
    }

    for (int i = 0; i < Pedalboard::kNumSlots; ++i)
    {
        const int place = state.getProperty ("place" + juce::String (i), 0);
        rig.pedalboard().setPlacement ((PedalType) i,
                                       (PedalPlacement) juce::jlimit (0, 2, place));
    }

    rig.metronome().setTempo ((double) state.getProperty ("metroBpm", 120.0));
    rig.metronome().setTimeSignature ((int) state.getProperty ("metroNum", 4),
                                      (int) state.getProperty ("metroDen", 4));
    rig.metronome().setSubdivision ((Subdivision) (int) state.getProperty ("metroSub", 0));
    rig.metronome().setSwing ((float) state.getProperty ("metroSwing", 0.0));
    rig.tuner().setReferenceA ((float) state.getProperty ("tunerRefA", 440.0));
    expressionCc = (int) state.getProperty ("expressionCc", 11);

    RigSettings r = rig.getSettings();
    r.oversampling = juce::jlimit (1, 4, (int) state.getProperty ("oversampling", 2));
    rig.setSettings (r);

    const juce::String irPath = state.getProperty ("userIrPath", juce::String()).toString();
    if (irPath.isNotEmpty())
    {
        juce::File f (irPath);
        if (f.existsAsFile()) loadUserIr (f);
    }

    rig.serviceMessageThread();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BassAmpProcessor();
}
