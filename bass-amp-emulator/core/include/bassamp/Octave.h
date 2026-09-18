// Octave.h - analog-style octave generation.
//
// Pitch-shifting DSP would be the obvious modern approach, but for bass it is
// the wrong one: any phase-vocoder or time-domain shifter adds latency and
// smears the attack. The analog trick is better here - square the fundamental
// with a Schmitt trigger, divide it with a flip-flop, then re-impose the input's
// envelope. That is what an OC-2 or an Ashdown sub-harmoniser does, it tracks in
// under a cycle, and it adds exactly zero samples of latency.
#pragma once

#include "Common.h"
#include "Filters.h"

namespace bassamp {

class OctaveDivider
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        // Strip everything above the fundamental region so the trigger sees one
        // clean zero crossing per cycle. Bass fundamentals run 31-400 Hz.
        track1.setLowpass (sampleRate, 260.0f, 0.707f);
        track2.setLowpass (sampleRate, 260.0f, 0.707f);
        smoothDown1.setLowpass (sampleRate, 900.0f, 0.707f);
        smoothDown2.setLowpass (sampleRate, 500.0f, 0.707f);
        // Fast attack so the octave tracks the pick transient, slow release so
        // the square-wave carrier does not amplitude-ripple at the note's own
        // frequency (|x| of a 41 Hz E has an 82 Hz ripple in it).
        envAttack  = timeConstantCoeff (0.0015f, sampleRate);
        envRelease = timeConstantCoeff (0.045f,  sampleRate);
        upBand.setBandpass (sampleRate, 700.0f, 0.6f);
        dcDown1.prepare (sampleRate);
        dcDown2.prepare (sampleRate);
        dcUp.prepare (sampleRate);
        reset();
    }

    void reset()
    {
        track1.reset(); track2.reset(); smoothDown1.reset(); smoothDown2.reset();
        upBand.reset(); dcDown1.reset(); dcDown2.reset(); dcUp.reset();
        env = 0.0f;
        state = false; flip1 = false; flip2 = false; lastAbove = false;
    }

    struct Out
    {
        float down1;   // one octave below
        float down2;   // two octaves below
        float up;      // one octave above (rectifier style)
    };

    inline Out process (float x)
    {
        const float tracked = track2.process (track1.process (x));
        const float rect = std::fabs (x);
        env = flushDenormal (rect > env ? envAttack  * env + (1.0f - envAttack)  * rect
                                        : envRelease * env + (1.0f - envRelease) * rect);

        // Schmitt trigger. The hysteresis scales with the envelope so it stays
        // immune to noise on a decaying note but still triggers on quiet ones.
        const float hyst = std::max (0.004f, env * 0.10f);
        if (! state && tracked >  hyst) state = true;
        if (  state && tracked < -hyst) state = false;

        if (state && ! lastAbove) { flip1 = ! flip1; if (flip1) flip2 = ! flip2; }
        lastAbove = state;

        const float sq1 = flip1 ? 1.0f : -1.0f;
        const float sq2 = flip2 ? 1.0f : -1.0f;

        Out out;
        out.down1 = dcDown1.process (smoothDown1.process (sq1 * env));
        out.down2 = dcDown2.process (smoothDown2.process (sq2 * env));
        // Full-wave rectification doubles the frequency; band-limiting it keeps
        // it from sounding like a fuzz box.
        out.up = dcUp.process (upBand.process (std::fabs (x) * 2.0f));
        return out;
    }

private:
    double fs = 48000.0;
    Biquad track1, track2, smoothDown1, smoothDown2, upBand;
    DCBlocker dcDown1, dcDown2, dcUp;
    float env = 0.0f, envAttack = 0.9f, envRelease = 0.99f;
    bool state = false, flip1 = false, flip2 = false, lastAbove = false;
};

} // namespace bassamp
