// Pedal.h - common interface for everything on the pedalboard.
//
// Every pedal exposes its controls through the same descriptor table, so the UI,
// the preset system and the plugin parameter tree are all generic: adding a new
// pedal means writing DSP and one descriptor array, nothing else.
#pragma once

#include "Common.h"
#include <memory>
#include <string>
#include <vector>

namespace bassamp {

enum class PedalType
{
    Compressor = 0,
    NoiseGate,
    Octave,
    Synth,
    Fuzz,
    Distortion,
    Overdrive,
    Wah,
    EnvelopeFilter,
    Chorus,
    Flanger,
    Phaser,
    Tremolo,
    RingMod,
    BitCrusher,
    GraphicEq,
    Delay,
    Reverb,
    NumTypes
};

struct ParamDesc
{
    const char* name = "Param";
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float defaultValue = 0.5f;
    const char* unit = "";
    int numChoices = 0;                        // 0 = continuous control
    const char* const* choices = nullptr;      // labels when numChoices > 0
    float skew = 0.5f;                         // < 0.5 gives resolution at the bottom
};

class Pedal
{
public:
    static constexpr int kMaxParams = 8;

    virtual ~Pedal() = default;

    virtual PedalType   type()        const = 0;
    virtual const char* name()        const = 0;
    virtual const char* description() const = 0;
    virtual int         numParams()   const = 0;
    virtual const ParamDesc& param (int index) const = 0;

    virtual void prepare (double sampleRate, int maxBlockSize) = 0;
    virtual void reset() = 0;
    virtual void process (float* data, int numSamples) = 0;

    // Modulation and delay pedals that offer tempo sync read this.
    virtual void setTempo (double bpm) { tempoBpm = bpm; }

    // Pedals that drive a meter in the UI (compressor, gate, envelope filter)
    // override this. Returns dB of gain reduction, or 0.
    virtual float getGainReductionDb() const { return 0.0f; }

    void  setParamValue (int index, float v);
    float getParamValue (int index) const;
    void  restoreDefaults();

    bool isEnabled() const { return enabled; }
    void setEnabled (bool e) { if (e != enabled) { enabled = e; if (! e) reset(); } }

protected:
    virtual void paramsChanged() {}

    float paramAt (int i) const { return values[i]; }
    int   choiceAt (int i) const { return (int) (values[i] + 0.5f); }
    bool  boolAt  (int i) const { return values[i] >= 0.5f; }

    // Standard note divisions for tempo-synced pedals.
    static float divisionToSeconds (int divisionIndex, double bpm);
    static const char* const* divisionChoices();
    static int divisionChoiceCount();

    float  values[kMaxParams] { };
    double fs = 48000.0;
    double tempoBpm = 120.0;
    int    maxBlock = 512;
    bool   enabled = false;
};

using PedalPtr = std::unique_ptr<Pedal>;

// Note divisions shared by every tempo-syncable pedal.
extern const char* const kDivisionNames[10];

PedalPtr createPedal (PedalType type);
const char* pedalTypeName (PedalType type);

// Boilerplate every concrete pedal needs. `descs` is defined in the .cpp.
#define BASSAMP_PEDAL_DECL(TypeEnum, DisplayName, Description)                       \
    PedalType type() const override { return TypeEnum; }                             \
    const char* name() const override { return DisplayName; }                         \
    const char* description() const override { return Description; }                  \
    int numParams() const override { return numDescs; }                               \
    const ParamDesc& param (int i) const override { return descs[i]; }                \
    static const ParamDesc descs[];                                                   \
    static const int numDescs;

// Companion definition, used once per pedal in the .cpp after its descs[] table.
#define BASSAMP_PEDAL_DEFS(Class) \
    const int Class::numDescs = (int) (sizeof (Class::descs) / sizeof (ParamDesc));

} // namespace bassamp
