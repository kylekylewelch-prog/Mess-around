// Cabinet.h - speaker cabinet + microphone simulation.
#pragma once

#include "Common.h"
#include "Convolver.h"
#include "IrDesigner.h"
#include "WavFile.h"
#include <atomic>
#include <string>
#include <vector>

namespace bassamp {

enum class CabType
{
    Bypass = 0,   // straight through - DI only
    C1x12,
    C1x15,
    C2x10,
    C2x12,
    C4x10,
    C4x12,
    C8x10,
    UserIr,
    NumTypes
};

enum class MicType
{
    DynamicLarge = 0,  // large-diaphragm kick mic: big low end, scooped low mids, presence lift
    DynamicSmall,      // classic stage dynamic: tighter lows, aggressive upper mids
    Condenser,         // extended top and bottom, most "hi-fi" of the four
    Ribbon,            // rolled-off top, thick lower mids
    NumTypes
};

struct CabinetSettings
{
    CabType cab        = CabType::C4x10;
    MicType mic        = MicType::DynamicLarge;
    float   micPosition = 0.35f;   // 0 = dead centre (bright), 1 = cone edge (dark)
    float   micDistance = 0.3f;    // 0 = on the grille, 1 = ~12 inches back
    float   roomAmount  = 0.15f;   // early reflections
    bool    hornEnabled = false;   // tweeter / horn, where the cab has one
    float   lowCutHz    = 30.0f;
    float   highCutHz   = 12000.0f;

    bool operator== (const CabinetSettings& o) const
    {
        return cab == o.cab && mic == o.mic
            && micPosition == o.micPosition && micDistance == o.micDistance
            && roomAmount == o.roomAmount && hornEnabled == o.hornEnabled
            && lowCutHz == o.lowCutHz && highCutHz == o.highCutHz;
    }
    bool operator!= (const CabinetSettings& o) const { return ! (*this == o); }
};

const char* cabName (CabType c);
const char* cabDescription (CabType c);
const char* micName (MicType m);
bool        cabHasHorn (CabType c);

// Builds the target magnitude response for a cabinet + mic combination.
std::vector<FilterSpec> buildCabinetVoicing (const CabinetSettings& s);

class Cabinet
{
public:
    static constexpr int kIrLength = 2048;      // 43 ms at 48 kHz - more than a cab needs
    static constexpr int kMaxUserIr = 8192;

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    // --- message thread -----------------------------------------------------
    // Designs the IR for `s` and hands it to the audio thread. Returns false if a
    // previous swap has not been consumed yet (caller should retry).
    bool applySettings (const CabinetSettings& s);

    // Loads a user IR file. Returns an empty string on success, otherwise the error.
    std::string loadUserIr (const std::string& path);
    bool hasUserIr() const { return userIrLoaded; }
    const std::string& getUserIrName() const { return userIrName; }

    // --- audio thread -------------------------------------------------------
    void process (float* data, int numSamples);

    // Magnitude response in dB (for the UI curve), relative to 110 Hz.
    float magnitudeDbAt (float freq) const;

private:
    void designInto (Convolver& target, const CabinetSettings& s);
    void applyTimeDomainCharacter (std::vector<float>& ir, const CabinetSettings& s) const;

    double fs = 48000.0;
    Convolver convA, convB;
    bool  activeIsA = true;

    std::atomic<bool> swapRequested { false };
    int   crossfadeCounter = 0, crossfadeLength = 0;
    std::vector<float> scratch;

    CabinetSettings current;
    std::vector<FilterSpec> currentVoicing;

    std::vector<float> userIr;
    double userIrRate = 48000.0;
    bool   userIrLoaded = false;
    std::string userIrName;

    Biquad lowCut, highCut;
};

} // namespace bassamp
