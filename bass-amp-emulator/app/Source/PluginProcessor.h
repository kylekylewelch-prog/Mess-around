#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "bassamp/Presets.h"
#include "ParameterBridge.h"
#include "TrackPlayer.h"

// Runs the tuner's pitch detection off the audio thread. YIN over a 90 ms window
// is far too much work to do inside a 64-sample callback, and it does not need to
// be there - the strobe display is what runs at audio rate.
class TunerAnalysisThread : public juce::Thread
{
public:
    explicit TunerAnalysisThread (bassamp::Tuner& t) : juce::Thread ("BassAmp tuner"), tuner (t) {}
    void run() override
    {
        while (! threadShouldExit())
        {
            tuner.runDetection();
            wait (20);
        }
    }

private:
    bassamp::Tuner& tuner;
};

class BassAmpProcessor : public juce::AudioProcessor,
                         private juce::Timer
{
public:
    BassAmpProcessor();
    ~BassAmpProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "BassAmp Studio"; }
    bool acceptsMidi() const override  { return true; }   // for expression-pedal control
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override { return juce::jmax (1, bassamp::numPresets()); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // --- accessors for the editor -------------------------------------------
    juce::AudioProcessorValueTreeState& getState() { return parameters; }
    bassamp::Rig& getRig() { return rig; }
    TrackPlayer& getTrackPlayer() { return trackPlayer; }

    void loadPreset (int index);
    juce::String loadUserIr (const juce::File& file);
    juce::String getUserIrName() const { return userIrName; }

    // Wah/filter expression control, driven by MIDI CC or the on-screen pedal.
    void setExpression (float value) { expression.store (juce::jlimit (0.0f, 1.0f, value)); }
    float getExpression() const { return expression.load(); }
    void setExpressionCc (int cc) { expressionCc = cc; }
    int  getExpressionCc() const { return expressionCc; }

    double getCurrentSampleRate() const { return currentSampleRate; }
    int    getCurrentBlockSize() const  { return currentBlockSize; }
    float  getCpuLoad() const { return cpuLoad.load(); }

private:
    void timerCallback() override;

    juce::AudioProcessorValueTreeState parameters;
    ParameterBridge bridge;
    bassamp::Rig rig;
    TrackPlayer trackPlayer;
    std::unique_ptr<TunerAnalysisThread> tunerThread;

    juce::AudioBuffer<float> monoInput;
    std::atomic<float> expression { 0.5f };
    std::atomic<float> cpuLoad { 0.0f };
    int expressionCc = 11;          // CC 11 (expression) by default, CC 4 also common
    int currentProgram = 0;
    double currentSampleRate = 48000.0;
    int currentBlockSize = 512;
    juce::String userIrName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassAmpProcessor)
};
