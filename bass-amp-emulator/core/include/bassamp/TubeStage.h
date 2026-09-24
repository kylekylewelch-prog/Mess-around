// TubeStage.h - the nonlinear building blocks the amps are made of.
//
// What actually makes a valve amp sound like a valve amp, in order of how much
// it matters for bass:
//
//   1. Asymmetry. A triode clips the two halves of the waveform differently, so
//      you get second harmonic, not just third. That is the "warmth".
//   2. Bias shift. Drive the grid hard enough and the coupling cap charges up,
//      pushing the operating point towards cutoff. The amp gets darker and
//      compresses, then recovers over tens of milliseconds. It is most of what
//      players describe as "feel".
//   3. Supply sag. A low B+ rail droops under load, which reduces headroom
//      dynamically. On a big note you hear the attack pass through clean and the
//      body compress behind it.
//   4. Output transformer behaviour, which for a bass amp mostly means low
//      frequency saturation - the reason a cranked SVT gets thick rather than
//      just loud.
//
// All four are here. None of them are expensive.
#pragma once

#include "Common.h"
#include "Filters.h"

namespace bassamp {

// Asymmetric soft clipper. The two asymptotes differ, which is where the even
// harmonics come from; the derivative at zero is 1 on both sides so the function
// stays C1 continuous and there is no crossover artefact at low level.
inline float asymmetricSoftClip (float x, float posHeadroom, float negHeadroom)
{
    return (x >= 0.0f) ? posHeadroom * fastTanh (x / posHeadroom)
                       : negHeadroom * fastTanh (x / negHeadroom);
}

struct TriodeStage
{
    // Voicing
    float gain          = 4.0f;     // stage gain before the nonlinearity
    float posHeadroom   = 0.75f;    // grid-conduction side: clips earlier
    float negHeadroom   = 1.35f;    // cutoff side: clips later and harder
    float biasShift     = 0.35f;    // 0 = no blocking distortion, 1 = very grabby
    float couplingHz    = 12.0f;    // inter-stage coupling cap
    float cathodeHz     = 90.0f;    // cathode bypass corner
    float cathodeBoostDb = 4.0f;    // gain above the cathode corner
    float millerHz      = 12000.0f; // grid-to-plate capacitance rolloff

    void prepare (double sampleRate)
    {
        fs = sampleRate;
        coupling.prepare (sampleRate);
        cathode.setLowShelf (sampleRate, cathodeHz, 0.7f, -cathodeBoostDb);
        miller.setLowpass (sampleRate, std::min (millerHz, (float) sampleRate * 0.45f), 0.707f);
        biasAttack  = timeConstantCoeff (0.004f, sampleRate);   // cap charges fast
        biasRelease = timeConstantCoeff (0.120f, sampleRate);   // and bleeds off slowly
        reset();
    }

    void reset() { coupling.reset(); cathode.reset(); miller.reset(); biasEnv = 0.0f; }

    inline float process (float x)
    {
        x = cathode.process (x) * gain;

        // Blocking distortion: track how far the positive peaks push past the
        // grid-conduction point and subtract that as a bias offset.
        const float over = std::max (0.0f, x - posHeadroom);
        biasEnv = flushDenormal (over > biasEnv ? biasAttack  * biasEnv + (1.0f - biasAttack)  * over
                                                : biasRelease * biasEnv + (1.0f - biasRelease) * over);
        x -= biasShift * biasEnv;

        float y = asymmetricSoftClip (x, posHeadroom, negHeadroom);
        y = miller.process (y);
        return coupling.process (y);   // the coupling cap also strips the DC the asymmetry created
    }

private:
    double fs = 48000.0;
    DCBlocker coupling;
    Biquad cathode, miller;
    float biasEnv = 0.0f, biasAttack = 0.9f, biasRelease = 0.99f;
};

// Solid-state gain stage: harder knee, symmetric, no bias shift. This is what a
// MAG-style FET preamp does when you push it, and it is deliberately less
// forgiving than the valve stage.
struct FetStage
{
    float gain = 3.0f;
    float knee = 0.85f;        // lower = harder clip
    float asymmetry = 1.05f;

    void prepare (double sampleRate)
    {
        blocker.prepare (sampleRate);
        post.setLowpass (sampleRate, std::min (14000.0f, (float) sampleRate * 0.45f), 0.707f);
        reset();
    }
    void reset() { blocker.reset(); post.reset(); }

    inline float process (float x)
    {
        x *= gain;
        const float p = knee, n = knee * asymmetry;
        // Cubic soft clip with a hard ceiling: more abrupt than tanh, which is
        // the point.
        auto shape = [] (float v, float lim)
        {
            const float a = v / lim;
            if (a >  1.5f) return  lim;
            if (a < -1.5f) return -lim;
            return lim * (a - (a * a * a) / 6.75f) * 1.0f;
        };
        const float y = (x >= 0.0f) ? shape (x, p) : shape (x, n);
        return blocker.process (post.process (y));
    }

private:
    DCBlocker blocker;
    Biquad post;
};

// Class AB push-pull output section with a sagging supply and a transformer.
struct PowerAmp
{
    float drive        = 1.0f;
    float headroom     = 1.0f;     // nominal rail
    float sagAmount    = 0.35f;    // 0 = stiff solid-state supply, 1 = tube rectifier
    float transformerLowHz  = 28.0f;
    float transformerHighHz = 9000.0f;
    float ironSaturation    = 0.5f; // low-frequency core saturation

    void prepare (double sampleRate)
    {
        fs = sampleRate;
        lowSplit.prepare (sampleRate, 140.0f);
        transformerHp.setHighpass (sampleRate, transformerLowHz, 0.9f);
        transformerLp.setLowpass  (sampleRate, std::min (transformerHighHz, (float) sampleRate * 0.45f), 0.8f);
        // Primary inductance resonance - the low-mid "push" of a valve amp
        // driving a real transformer.
        resonance.setPeaking (sampleRate, 78.0f, 1.1f, 2.0f);
        sagAttack  = timeConstantCoeff (0.012f, sampleRate);
        sagRelease = timeConstantCoeff (0.180f, sampleRate);
        reset();
    }

    void reset()
    {
        lowSplit.reset(); transformerHp.reset(); transformerLp.reset(); resonance.reset();
        sagEnv = 0.0f;
    }

    inline float process (float x)
    {
        x = resonance.process (x) * drive;

        // Supply sag: the rail collapses under sustained current draw, so the
        // available headroom shrinks behind the attack transient.
        const float rect = std::fabs (x);
        sagEnv = flushDenormal (rect > sagEnv ? sagAttack  * sagEnv + (1.0f - sagAttack)  * rect
                                              : sagRelease * sagEnv + (1.0f - sagRelease) * rect);
        const float rail = headroom * (1.0f - sagAmount * 0.55f * clampf (sagEnv, 0.0f, 1.2f));

        // Push-pull is close to symmetric, with a small residual imbalance.
        float y = asymmetricSoftClip (x, rail, rail * 1.04f);

        // Transformer core saturation acts on flux, which is the integral of
        // voltage - so it is the low end that saturates first. Split, saturate
        // the lows harder, recombine.
        float low, high;
        lowSplit.process (y, low, high);
        low = asymmetricSoftClip (low, 1.0f - 0.45f * ironSaturation, 1.0f - 0.40f * ironSaturation);
        y = low + high;

        return transformerLp.process (transformerHp.process (y));
    }

    float getSagDb() const { return gainToDb (1.0f - sagAmount * 0.55f * clampf (sagEnv, 0.0f, 1.2f)); }

private:
    double fs = 48000.0;
    LinkwitzRiley4 lowSplit;
    Biquad transformerHp, transformerLp, resonance;
    float sagEnv = 0.0f, sagAttack = 0.9f, sagRelease = 0.99f;
};

} // namespace bassamp
