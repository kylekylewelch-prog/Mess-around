// Pedals.h - the concrete pedal models.
#pragma once

#include "Pedal.h"
#include "Filters.h"
#include "DelayLine.h"
#include "Octave.h"
#include <array>

namespace bassamp {

// --- dynamics ---------------------------------------------------------------

class CompressorPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Compressor, "Compressor",
        "Feed-forward compressor with three detector characters. Opto is slow and "
        "program dependent - it evens out a line without you hearing it work. FET "
        "grabs the transient hard, for a modern finger style. Bass mode side-chains "
        "off the mids so low notes do not duck the whole signal.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;
    float getGainReductionDb() const override { return -currentReductionDb; }

private:
    void paramsChanged() override;
    Biquad sidechainHp;
    float envDb = 0.0f, currentReductionDb = 0.0f;
    float attackCoeff = 0.9f, releaseCoeff = 0.99f;
};

class NoiseGatePedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::NoiseGate, "Noise Gate",
        "Hysteresis gate placed first in the chain. Single-coil hum and amp hiss "
        "get worse as you add gain, so this shuts the chain down between phrases "
        "without chopping note tails.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;
    float getGainReductionDb() const override { return gainToDb (currentGain); }

private:
    void paramsChanged() override;
    EnvelopeFollower detector;
    float currentGain = 1.0f, attackCoeff = 0.9f, releaseCoeff = 0.999f;
    int   holdCounter = 0, holdSamples = 0;
    bool  open = false;
};

class GraphicEqPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::GraphicEq, "Graphic EQ",
        "Seven fixed bands at the frequencies that matter on bass. Use it after the "
        "amp to fix a room, or before it to change what the preamp is being asked "
        "to distort.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;
    float magnitudeDbAt (float freq) const;

private:
    void paramsChanged() override;
    static constexpr int kBands = 7;
    Biquad bands[kBands];
};

// --- pitch ------------------------------------------------------------------

class OctavePedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Octave, "Octave",
        "Analog-style divider: one and two octaves down plus a rectifier octave up. "
        "Tracks in well under a cycle and adds no latency, which is why this "
        "approach still beats pitch shifting on bass. Play single notes above the "
        "5th fret for the cleanest tracking.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    void paramsChanged() override;
    OctaveDivider divider;
    Biquad toneLp;
};

class SynthPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Synth, "Bass Synth",
        "Monophonic synth voice locked to the note you play. A phase accumulator is "
        "re-triggered on each cycle of the fundamental, so the oscillator is in tune "
        "and in phase with the string with no detection delay. Envelope-swept "
        "resonant filter on top.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    void paramsChanged() override;

    Biquad trackLp1, trackLp2;
    SvfTPT filter;
    EnvelopeFollower amplitude, sweepEnv;
    DCBlocker outputDc;
    OctaveDivider divider;

    float oscPhase = 0.0f, oscInc = 0.0f;
    float periodEstimate = 480.0f;
    int   samplesSinceCross = 0;
    bool  above = false;
    float trigHysteresis = 0.01f;
};

// --- dirt -------------------------------------------------------------------

// Shared low-band bypass. Distorting the fundamental of a low B is what makes a
// bass fuzz disappear in a band mix; every dirt pedal here can keep the bottom
// clean and only dirty the band above the crossover.
struct CleanLowSplit
{
    void prepare (double fs) { crossover.prepare (fs, 120.0f); sampleRate = fs; }
    void setFrequency (float hz) { crossover.prepare (sampleRate, clampf (hz, 40.0f, 600.0f)); }
    void reset() { crossover.reset(); }
    LinkwitzRiley4 crossover;
    double sampleRate = 48000.0;
};

class FuzzPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Fuzz, "Fuzz",
        "Two cascaded clipping stages with the classic scooped tone network. The "
        "Low Keep control is the important one on bass: everything below it bypasses "
        "the fuzz entirely, so you keep your fundamental and the fuzz sits on top "
        "instead of swallowing the note.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    void paramsChanged() override;
    CleanLowSplit split;
    Biquad stage1Hp, stage2Hp, toneLp, toneHp, postLp;
    DCBlocker dc1, dc2;
};

class DistortionPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Distortion, "Distortion",
        "Hard-knee diode clipper with a pre-emphasis stage. More aggressive and "
        "more compressed than the overdrive, with the grind sitting in the upper "
        "mids where a bass cuts through a guitar.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    void paramsChanged() override;
    CleanLowSplit split;
    Biquad preEmphasis, toneTilt, postLp;
    DCBlocker dc;
};

class OverdrivePedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Overdrive, "Overdrive",
        "Op-amp clipping inside a feedback loop, with the high-pass before the "
        "diodes that gives this circuit its mid hump. Naturally bass-friendly: the "
        "low end goes round the clipping stage rather than through it.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    void paramsChanged() override;
    Biquad loopHp, midHump, toneLp;
    DCBlocker dc;
};

class BitCrusherPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::BitCrusher, "Bit Crusher",
        "Bit-depth and sample-rate reduction. Quantisation noise here is deliberate "
        "and correlated with the signal - that is the sound.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    float held = 0.0f, phase = 0.0f;
};

class RingModPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::RingMod, "Ring Mod",
        "Multiplies the input by a carrier, producing sum and difference "
        "frequencies. Inharmonic by nature - keep the mix low and it works as a "
        "texture rather than a gimmick.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    Lfo carrier;
};

// --- filters ----------------------------------------------------------------

class WahPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Wah, "Wah",
        "Resonant band-pass sweep with the inductor peak of the original circuit. "
        "Drive it from the Pedal control (assign it to a MIDI expression pedal) or "
        "let the LFO sweep it.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    void paramsChanged() override;
    SvfTPT svf;
    Lfo lfo;
    Biquad outputTilt;
    SmoothedValue pedalSmooth;
};

class EnvelopeFilterPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::EnvelopeFilter, "Envelope Filter",
        "The funk staple. Playing dynamics sweep the filter, so the control is in "
        "your right hand. Down mode sweeps backwards, which is the sound on a lot "
        "of the records people are trying to copy when they buy one of these.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;
    float getGainReductionDb() const override { return sweepDisplay; }

private:
    void paramsChanged() override;
    SvfTPT svf;
    EnvelopeFollower follower;
    float sweepDisplay = 0.0f;
};

// --- modulation -------------------------------------------------------------

class ChorusPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Chorus, "Chorus",
        "Up to three modulated voices at different LFO phases. On bass it works "
        "best with the low end kept dry - the Low Keep control does that, so the "
        "fundamental stays solid while the harmonics shimmer.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    void paramsChanged() override;
    DelayLine line;
    Lfo lfo;
    CleanLowSplit split;
};

class FlangerPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Flanger, "Flanger",
        "Short modulated delay with feedback. Negative feedback gives the hollow, "
        "through-zero-ish sweep; positive gives the classic jet.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    void paramsChanged() override;
    DelayLine line;
    Lfo lfo;
    Biquad feedbackLp;
    float feedbackState = 0.0f;
};

class PhaserPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Phaser, "Phaser",
        "Cascaded all-pass stages swept by an LFO. Unlike a flanger the notches are "
        "not harmonically spaced, so it stays out of the way of the fundamental.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    void paramsChanged() override;
    static constexpr int kMaxStages = 12;
    Biquad stages[kMaxStages];
    Lfo lfo;
    float feedbackState = 0.0f;
};

class TremoloPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Tremolo, "Tremolo",
        "Amplitude modulation, tempo-syncable. Square shape gives the chopped "
        "stutter; sine is the amp-style throb.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    void paramsChanged() override;
    Lfo lfo;
};

// --- time -------------------------------------------------------------------

class DelayPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Delay, "Delay",
        "Digital, analog and tape characters. Analog darkens each repeat; tape adds "
        "wow and flutter plus a little saturation, so the repeats drift and blur the "
        "way a real machine does.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    void paramsChanged() override;
    DelayLine line;
    Biquad repeatLp, repeatHp;
    Lfo wow, flutter;
    SmoothedValue timeSmooth;
    float feedbackState = 0.0f;
};

class ReverbPedal : public Pedal
{
public:
    BASSAMP_PEDAL_DECL (PedalType::Reverb, "Reverb",
        "Feedback delay network with input diffusion. Kept deliberately modest: on "
        "bass, reverb is an ambience control, not an effect.")

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void process (float* data, int numSamples) override;

private:
    void paramsChanged() override;
    static constexpr int kLines = 4;
    DelayLine lines[kLines];
    DelayLine preDelay;
    DelayLine diffusion[2];
    Biquad damping[kLines];
    Biquad inputHp;
    float feedbackState[kLines] { };
    float lineLengths[kLines] { };
};

} // namespace bassamp
