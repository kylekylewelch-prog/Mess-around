// IrDesigner.h - builds cabinet impulse responses from a voicing description.
//
// Shipping third-party cabinet IRs means shipping someone else's copyrighted
// recordings, so the cabinets here are *designed* rather than sampled: each one
// is described as a target magnitude response (driver rolloffs, port/sealed
// alignment, cone breakup, horn) plus a mic and a small amount of room, and that
// description is converted into a minimum-phase impulse response at load time.
//
// Minimum phase is the right choice for a cab: it puts all the energy at the
// front of the response, so the convolver's head section does most of the work
// and there is no pre-ringing. Users who want a captured cab can load their own
// IR - the convolution path is identical.
#pragma once

#include "Common.h"
#include "Filters.h"
#include "Fft.h"
#include <vector>

namespace bassamp {

struct FilterSpec
{
    enum class Type { LowShelf, HighShelf, Peak, LowPass, HighPass, Notch };

    Type  type  = Type::Peak;
    float freq  = 1000.0f;
    float q     = 0.707f;
    float gainDb = 0.0f;
    int   repeats = 1;   // cascade the same section N times for a steeper slope
};

class IrDesigner
{
public:
    // Evaluates the magnitude response of a voicing at an arbitrary frequency.
    static float magnitudeOf (const std::vector<FilterSpec>& specs, float freq, double sampleRate);

    // Converts a target magnitude response into a causal minimum-phase impulse
    // response of `irLength` samples, via the real-cepstrum method.
    static void designMinimumPhase (const std::vector<FilterSpec>& specs,
                                    double sampleRate,
                                    int irLength,
                                    std::vector<float>& out);

    // Normalises to unity gain at a reference frequency so switching cabinets
    // does not change perceived level.
    static void normaliseAt (std::vector<float>& ir, double sampleRate, float refFreq = 110.0f);
};

} // namespace bassamp
