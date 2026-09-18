// Amp.h - the amplifier models.
#pragma once

#include "Common.h"
#include "Filters.h"
#include "TubeStage.h"
#include "Octave.h"
#include "Oversampler.h"
#include <vector>

namespace bassamp {

enum class AmpType
{
    AshdownMag = 0,   // MAG-series solid state head: FET preamp, shape switch, sub harmoniser
    AmpegSvt,         // all-valve 300 W: selectable midrange, Ultra Lo / Ultra Hi
    AmpegB15,         // flip-top Portaflex: low headroom, compresses early, simple tone
    FenderBassman,    // 5F6-A tweed: interactive passive stack with its famous mid scoop
    StudioDI,         // transformer-coupled studio DI, effectively clean
    DriverDI,         // bass driver DI preamp: blend, drive, presence, built-in speaker sim
    NumTypes
};

const char* ampName (AmpType t);
const char* ampDescription (AmpType t);
// True when the amp actually has the control - the UI greys out the rest rather
// than pretending a Bassman has an Ultra Lo switch.
bool ampHasMidFreq   (AmpType t);
bool ampHasPresence  (AmpType t);
bool ampHasUltra     (AmpType t);
bool ampHasShape     (AmpType t);
bool ampHasSub       (AmpType t);
bool ampHasBlend     (AmpType t);
bool ampHasBright    (AmpType t);
// Centre frequencies offered by the amp's midrange selector.
int   ampMidFreqCount (AmpType t);
float ampMidFreqValue (AmpType t, int index);

struct AmpSettings
{
    AmpType type = AmpType::AmpegSvt;

    float gain     = 0.5f;   // preamp drive
    float master   = 0.6f;   // power amp drive
    float output   = 0.7f;   // post level
    float bass     = 0.5f;
    float mid      = 0.5f;
    float treble   = 0.5f;
    int   midFreqIndex = 2;  // index into the amp's midrange selector
    float presence = 0.3f;
    float blend    = 1.0f;   // DriverDI: dry/driven blend
    float subLevel = 0.0f;   // AshdownMag: sub harmoniser level
    bool  bright   = false;
    bool  ultraLo  = false;
    bool  ultraHi  = false;
    bool  shape    = false;
    float sag      = 0.5f;   // supply stiffness, exposed because it is the single
                             // biggest "feel" control in the whole rig
};

class Amp
{
public:
    void prepare (double sampleRate, int maxBlockSize, int oversampleFactor);
    void setOversampling (int factor);
    void reset();

    void setSettings (const AmpSettings& s);
    const AmpSettings& getSettings() const { return settings; }

    void process (float* data, int numSamples);

    float getSagDb()   const { return power.getSagDb(); }
    float getDriveDb() const { return gainToDb (driveMeter); }

    // Tone-stack magnitude in dB at a frequency, for the UI response curve.
    float toneMagnitudeDbAt (float freq) const;

private:
    void updateVoicing();

    double baseRate = 48000.0, osRate = 48000.0;
    int    maxBlock = 512, osFactor = 2;

    AmpSettings settings;

    Oversampler oversampler;

    // Fixed input/output conditioning (base rate)
    Biquad inputHp, outputLp;

    // Tone stack: six sections is enough for every model here.
    static constexpr int kToneSections = 12;
    Biquad tone[kToneSections];
    int    toneCount = 0;
    float  makeupGain = 1.0f;

    Biquad brightShelf, presenceShelf;
    bool   brightActive = false, presenceActive = false;

    TriodeStage triode1, triode2;
    FetStage    fet1, fet2;
    PowerAmp    power;
    OctaveDivider sub;
    Biquad      subLp;
    float       preGainTarget = 1.0f;
    DCBlocker   postDc;

    SmoothedValue gainSmooth, masterSmooth, outputSmooth, blendSmooth, subSmooth;
    float driveMeter = 0.0f, driveRelease = 0.999f;

    std::vector<float> dryBuffer;
};

} // namespace bassamp
