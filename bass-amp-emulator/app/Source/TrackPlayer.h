// TrackPlayer.h - backing-track playback for practising along with.
//
// The track is mixed in after the rig's master, so it is monitored but never
// processed - the fuzz should not be eating the drums. Reading happens on a
// background thread so a disk hiccup cannot cause an audio dropout.
#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "bassamp/Rig.h"

class TrackPlayer
{
public:
    TrackPlayer();
    ~TrackPlayer();

    void prepare (double sampleRate, int maxBlockSize);
    void releaseResources();

    // Message thread. Returns an empty string on success.
    juce::String loadFile (const juce::File& file);
    void unload();

    void play();
    void stop();
    void togglePlay();
    bool isPlaying() const;

    void setPosition (double seconds);
    double getPosition() const;
    double getLength() const;

    void setLooping (bool shouldLoop);
    bool isLooping() const { return looping; }

    void setLevelDb (float db)   { levelDb = db; }
    float getLevelDb() const     { return levelDb; }
    void setBassCut (float amount) { bassCut = juce::jlimit (0.0f, 1.0f, amount); }
    float getBassCut() const     { return bassCut; }

    juce::String getFileName() const { return fileName; }
    bool hasFile() const { return readerSource != nullptr; }

    // Audio thread: adds the track into the output buffer.
    void mixInto (float* left, float* right, int numSamples);

private:
    juce::AudioFormatManager formats;
    juce::TimeSliceThread readThread { "BassAmp track reader" };
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transport;
    juce::AudioBuffer<float> scratch;

    bassamp::BackingTrackProcessor processor;

    juce::String fileName;
    std::atomic<float> levelDb { -6.0f };
    std::atomic<float> bassCut { 0.0f };
    bool looping = true;
    bool prepared = false;
};
