#include "TrackPlayer.h"

TrackPlayer::TrackPlayer()
{
    formats.registerBasicFormats();   // WAV, AIFF, plus MP3/Ogg/FLAC where enabled
    readThread.startThread (juce::Thread::Priority::normal);
}

TrackPlayer::~TrackPlayer()
{
    transport.setSource (nullptr);
    readThread.stopThread (2000);
}

void TrackPlayer::prepare (double sampleRate, int maxBlockSize)
{
    transport.prepareToPlay (maxBlockSize, sampleRate);
    scratch.setSize (2, juce::jmax (1, maxBlockSize));
    processor.prepare (sampleRate);
    prepared = true;
}

void TrackPlayer::releaseResources()
{
    transport.releaseResources();
    prepared = false;
}

juce::String TrackPlayer::loadFile (const juce::File& file)
{
    if (! file.existsAsFile()) return "File does not exist";

    std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (file));
    if (reader == nullptr)
        return "Unsupported audio format (try WAV, AIFF, FLAC or MP3)";

    transport.stop();
    transport.setSource (nullptr);

    auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader.release(), true);
    newSource->setLooping (looping);
    transport.setSource (newSource.get(), 32768, &readThread, newSource->getAudioFormatReader()->sampleRate);
    readerSource = std::move (newSource);

    fileName = file.getFileName();
    transport.setPosition (0.0);
    return {};
}

void TrackPlayer::unload()
{
    transport.stop();
    transport.setSource (nullptr);
    readerSource.reset();
    fileName.clear();
}

void TrackPlayer::play()   { if (readerSource != nullptr) transport.start(); }
void TrackPlayer::stop()   { transport.stop(); }
void TrackPlayer::togglePlay() { if (isPlaying()) stop(); else play(); }
bool TrackPlayer::isPlaying() const { return transport.isPlaying(); }

void TrackPlayer::setPosition (double seconds) { transport.setPosition (seconds); }
double TrackPlayer::getPosition() const { return transport.getCurrentPosition(); }
double TrackPlayer::getLength() const   { return transport.getLengthInSeconds(); }

void TrackPlayer::setLooping (bool shouldLoop)
{
    looping = shouldLoop;
    if (readerSource != nullptr) readerSource->setLooping (shouldLoop);
}

void TrackPlayer::mixInto (float* left, float* right, int numSamples)
{
    if (! prepared || readerSource == nullptr || ! transport.isPlaying()) return;
    if (scratch.getNumSamples() < numSamples) scratch.setSize (2, numSamples, false, false, true);

    juce::AudioBuffer<float> view (scratch.getArrayOfWritePointers(), 2, numSamples);
    view.clear();

    juce::AudioSourceChannelInfo info (&view, 0, numSamples);
    transport.getNextAudioBlock (info);

    float* l = view.getWritePointer (0);
    float* r = view.getWritePointer (1);
    processor.process (l, r, numSamples, bassamp::dbToGain (levelDb.load()), bassCut.load());

    for (int n = 0; n < numSamples; ++n)
    {
        left[n]  += l[n];
        right[n] += r[n];
    }
}
