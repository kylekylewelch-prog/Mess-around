#include "bassamp/Presets.h"
#include <map>

namespace bassamp {

namespace {

// Small builders so the preset table below reads like a settings sheet rather
// than a wall of struct initialisers.
AmpSettings ampOf (AmpType type, float gain, float master, float bass, float mid, float treble,
                   float presence = 0.3f, int midFreq = 2, float sag = 0.5f)
{
    AmpSettings a;
    a.type = type;
    a.gain = gain; a.master = master; a.output = 0.7f;
    a.bass = bass; a.mid = mid; a.treble = treble;
    a.midFreqIndex = midFreq;
    a.presence = presence;
    a.sag = sag;
    return a;
}

CabinetSettings cabOf (CabType cab, MicType mic, float position, float distance,
                       float room = 0.12f, bool horn = false)
{
    CabinetSettings c;
    c.cab = cab; c.mic = mic;
    c.micPosition = position; c.micDistance = distance;
    c.roomAmount = room; c.hornEnabled = horn;
    return c;
}

RigSettings rigOf (float input, float master, float di)
{
    RigSettings r;
    r.inputGainDb = input;
    r.masterLevelDb = master;
    r.diBlend = di;
    return r;
}

PedalPreset ped (PedalType type, std::vector<float> params,
                 PedalPlacement placement = PedalPlacement::FrontOfAmp)
{
    return { type, placement, std::move (params) };
}

std::vector<Preset> buildPresets()
{
    std::vector<Preset> p;

    // ---- Soul / Motown / R&B ------------------------------------------------
    p.push_back ({
        "Detroit Thumb", "Soul / Motown",
        "Flip-top valve head into a 1x15, large dynamic mic close and slightly off "
        "centre. The Motown session sound: a Precision with flatwounds and a foam "
        "mute under the bridge, played with one finger.",
        "Almost all fundamental. Roll your tone control right off and play over the "
        "end of the fingerboard. If it sounds too dull soloed, it is probably right "
        "in the track.",
        ampOf (AmpType::AmpegB15, 0.42f, 0.55f, 0.66f, 0.50f, 0.22f, 0.0f, 2, 0.75f),
        cabOf (CabType::C1x15, MicType::DynamicLarge, 0.45f, 0.20f, 0.10f),
        rigOf (0.0f, -3.0f, 0.15f),
        { ped (PedalType::Compressor, { -20.0f, 3.0f, 18.0f, 220.0f, 5.0f, 100.0f, 0.0f }) }
    });

    p.push_back ({
        "Memphis Pocket", "Soul / Motown",
        "Flip-top with a touch more grind and a ribbon mic further back - the Stax "
        "rhythm section sound, where the bass sits behind the beat and glues the "
        "horns to the drums.",
        "Play slightly behind the click. The amp is close enough to breakup that "
        "digging in changes the tone, which is the point.",
        ampOf (AmpType::AmpegB15, 0.55f, 0.62f, 0.60f, 0.55f, 0.30f, 0.0f, 2, 0.8f),
        cabOf (CabType::C1x15, MicType::Ribbon, 0.35f, 0.45f, 0.20f),
        rigOf (1.0f, -3.0f, 0.10f),
        { ped (PedalType::Compressor, { -18.0f, 4.0f, 25.0f, 260.0f, 6.0f, 100.0f, 0.0f }) }
    });

    p.push_back ({
        "Modern R&B DI", "Soul / Motown",
        "Transformer studio DI, no speaker. What a modern R&B or neo-soul record "
        "actually prints: clean, controlled, all the weight in the fundamental.",
        "Compression does the work here. Keep your right-hand dynamics even.",
        ampOf (AmpType::StudioDI, 0.25f, 0.5f, 0.60f, 0.45f, 0.55f, 0.0f, 1, 0.0f),
        cabOf (CabType::Bypass, MicType::Condenser, 0.0f, 0.0f, 0.0f),
        rigOf (0.0f, -4.0f, 1.0f),
        { ped (PedalType::Compressor, { -22.0f, 4.0f, 8.0f, 150.0f, 7.0f, 100.0f, 0.0f }),
          ped (PedalType::GraphicEq, { 1.0f, 2.0f, -2.0f, -1.0f, 0.0f, 1.5f, 1.0f }) }
    });

    // ---- Rock ---------------------------------------------------------------
    p.push_back ({
        "Fridge Standard", "Rock",
        "All-valve 300 W head into a sealed 8x10 with a large dynamic mic on the "
        "grille. The default rock bass rig for fifty years, and still the one that "
        "sits in a loud band without any help.",
        "Gain around two thirds is the sweet spot: clean on light playing, thick "
        "when you dig in. The midrange selector is the most useful control on the "
        "amp - move it, do not just boost it.",
        ampOf (AmpType::AmpegSvt, 0.55f, 0.72f, 0.58f, 0.55f, 0.55f, 0.35f, 2, 0.5f),
        cabOf (CabType::C8x10, MicType::DynamicLarge, 0.30f, 0.25f, 0.12f),
        rigOf (0.0f, -3.0f, 0.12f),
        { ped (PedalType::Compressor, { -16.0f, 3.0f, 15.0f, 200.0f, 4.0f, 60.0f, 2.0f }) }
    });

    p.push_back ({
        "Pick & Grind", "Rock",
        "Valve head pushed into breakup, 4x10 with the horn on and a small dynamic "
        "mic close in. Picked rock bass: the attack has to cut through two guitars.",
        "Play with a pick near the bridge. Most of the aggression here is the "
        "upper mids, not the distortion.",
        ampOf (AmpType::AmpegSvt, 0.78f, 0.80f, 0.55f, 0.68f, 0.62f, 0.55f, 2, 0.45f),
        cabOf (CabType::C4x10, MicType::DynamicSmall, 0.20f, 0.20f, 0.10f, true),
        rigOf (1.0f, -4.0f, 0.10f),
        { ped (PedalType::Compressor, { -14.0f, 3.0f, 8.0f, 160.0f, 3.0f, 55.0f, 1.0f }),
          ped (PedalType::Overdrive,  { 30.0f, 55.0f, -4.0f, 60.0f }) }
    });

    p.push_back ({
        "Tweed Roundhouse", "Rock",
        "Tweed-era valve amp through a 2x12. The mid scoop of the passive tone "
        "stack plus early power-amp breakup: loose, warm and vintage.",
        "Back the mid control off for the classic scoop, or push it up past halfway "
        "and the amp turns into a very different, much more aggressive thing.",
        ampOf (AmpType::FenderBassman, 0.62f, 0.75f, 0.62f, 0.38f, 0.58f, 0.40f, 2, 0.6f),
        cabOf (CabType::C2x12, MicType::Ribbon, 0.30f, 0.35f, 0.18f),
        rigOf (0.0f, -4.0f, 0.08f),
        { }
    });

    p.push_back ({
        "Ricken Growl", "Rock",
        "Bridge-pickup roundwound growl into a valve head and a 4x12, with the two "
        "classic tricks: a high-pass on the dirt so the low end stays clean, and "
        "aggressive upper mids.",
        "Bridge pickup, fresh roundwounds, pick near the bridge. This is a "
        "midrange sound - resist the urge to add bass.",
        ampOf (AmpType::AmpegSvt, 0.70f, 0.78f, 0.45f, 0.75f, 0.70f, 0.60f, 3, 0.4f),
        cabOf (CabType::C4x12, MicType::DynamicSmall, 0.15f, 0.25f, 0.12f),
        rigOf (1.0f, -4.0f, 0.15f),
        { ped (PedalType::Distortion, { 35.0f, 4.0f, -8.0f, 160.0f, 55.0f }),
          ped (PedalType::GraphicEq,  { -1.0f, -1.0f, 2.0f, 4.0f, 3.0f, 1.0f, 0.0f }) }
    });

    p.push_back ({
        "Loud & Dirty", "Rock",
        "Bass rig run like a guitar rig: treble and bass up, mids present, power "
        "amp flat out. Almost a rhythm-guitar tone with a low B under it.",
        "This sound only works as the only midrange instrument in the band, or as "
        "a deliberate lead bass part.",
        ampOf (AmpType::FenderBassman, 0.92f, 0.92f, 0.78f, 0.55f, 0.82f, 0.75f, 2, 0.55f),
        cabOf (CabType::C4x12, MicType::DynamicSmall, 0.10f, 0.15f, 0.08f),
        rigOf (2.0f, -6.0f, 0.05f),
        { ped (PedalType::Overdrive, { 65.0f, 62.0f, -6.0f, 100.0f }) }
    });

    // ---- Metal --------------------------------------------------------------
    p.push_back ({
        "Split Rig Metal", "Metal",
        "The modern metal approach: a clean low band under a heavily distorted "
        "upper band, so the bass keeps its weight while the grind rides on top of "
        "the guitars. Here the crossover is inside the distortion.",
        "Low Keep at 120 Hz is the important control. Drop it and the low end "
        "disappears into the guitars; raise it and the grind loses its bite.",
        ampOf (AmpType::AmpegSvt, 0.68f, 0.82f, 0.62f, 0.62f, 0.70f, 0.60f, 3, 0.35f),
        cabOf (CabType::C8x10, MicType::DynamicSmall, 0.18f, 0.18f, 0.08f),
        rigOf (1.0f, -5.0f, 0.18f),
        { ped (PedalType::NoiseGate,  { -48.0f, 1.0f, 40.0f, 150.0f, 60.0f }),
          ped (PedalType::Compressor, { -18.0f, 4.0f, 6.0f, 140.0f, 5.0f, 70.0f, 1.0f }),
          ped (PedalType::Distortion, { 72.0f, 5.0f, -9.0f, 120.0f, 85.0f }) }
    });

    p.push_back ({
        "Lead Bass Fuzz", "Metal",
        "Fuzz and wah in front of a cranked valve amp - the lead-bass sound, where "
        "the bass is carrying the melody rather than the bottom.",
        "Switch the wah to pedal mode and assign it to an expression pedal. The "
        "fuzz's Low Keep is set high so the fuzz sits above the fundamental.",
        ampOf (AmpType::AmpegSvt, 0.75f, 0.85f, 0.55f, 0.70f, 0.65f, 0.55f, 2, 0.45f),
        cabOf (CabType::C4x12, MicType::DynamicSmall, 0.20f, 0.20f, 0.10f),
        rigOf (1.0f, -6.0f, 0.10f),
        { ped (PedalType::Fuzz, { 82.0f, 55.0f, -10.0f, 150.0f, 85.0f }),
          ped (PedalType::Wah,  { 50.0f, 300.0f, 1800.0f, 5.0f, 0.0f, 1.2f, 85.0f }) }
    });

    p.push_back ({
        "Down-Tuned Djent", "Metal",
        "Tight, fast, aggressive. Heavy compression into a hard distortion with the "
        "low end protected, then a mid scoop to leave room for the guitars.",
        "Built for a 5-string tuned down. Play close to the bridge with a light "
        "touch - the compressor supplies the consistency.",
        ampOf (AmpType::AshdownMag, 0.55f, 0.75f, 0.62f, 0.45f, 0.72f, 0.50f, 3, 0.2f),
        cabOf (CabType::C4x10, MicType::DynamicSmall, 0.15f, 0.15f, 0.06f, true),
        rigOf (1.0f, -5.0f, 0.30f),
        { ped (PedalType::NoiseGate,  { -46.0f, 0.5f, 30.0f, 120.0f, 70.0f }),
          ped (PedalType::Compressor, { -22.0f, 8.0f, 2.0f, 90.0f, 8.0f, 85.0f, 1.0f }),
          ped (PedalType::Distortion, { 80.0f, 8.0f, -10.0f, 140.0f, 90.0f }),
          ped (PedalType::GraphicEq,  { 2.0f, 1.0f, -4.0f, -2.0f, 2.0f, 4.0f, 2.0f }) }
    });

    // ---- Punk ---------------------------------------------------------------
    p.push_back ({
        "Downstroke Punk", "Punk",
        "Precision bass with a pick, flat out into a valve head and an 8x10. No "
        "pedals, nothing clever - all the aggression comes from the right hand and "
        "the amp being at the edge of breakup.",
        "All downstrokes, hard, near the bridge pickup. Mids up, not down.",
        ampOf (AmpType::AmpegSvt, 0.72f, 0.85f, 0.52f, 0.72f, 0.68f, 0.55f, 2, 0.4f),
        cabOf (CabType::C8x10, MicType::DynamicSmall, 0.22f, 0.18f, 0.10f),
        rigOf (2.0f, -4.0f, 0.10f),
        { }
    });

    // ---- Funk ---------------------------------------------------------------
    p.push_back ({
        "Envelope Funk", "Funk",
        "Envelope filter into a valve amp and a 4x10 with the horn on. The filter "
        "is driven by your right hand, so dynamics are the instrument.",
        "Play muted sixteenths quietly and the filter stays shut; pop a note and it "
        "opens. If it is triggering on everything, turn Sensitivity down, not up.",
        ampOf (AmpType::AmpegSvt, 0.45f, 0.62f, 0.55f, 0.58f, 0.62f, 0.45f, 2, 0.4f),
        cabOf (CabType::C4x10, MicType::Condenser, 0.25f, 0.25f, 0.12f, true),
        rigOf (0.0f, -4.0f, 0.20f),
        { ped (PedalType::Compressor,     { -18.0f, 4.0f, 10.0f, 140.0f, 5.0f, 80.0f, 0.0f }),
          ped (PedalType::EnvelopeFilter, { 62.0f, 240.0f, 6.0f, 6.0f, 150.0f, 1.0f, 0.0f, 100.0f }) }
    });

    p.push_back ({
        "Slap & Pop", "Funk",
        "Scooped, bright and compressed: thumb on the low strings, fingers popping "
        "the top. The EQ curve here is the classic smile, which works for slap and "
        "for almost nothing else.",
        "Raise the action slightly if you are fighting fret buzz. The compressor is "
        "doing a lot - back the Mix off if it sounds squashed.",
        ampOf (AmpType::AshdownMag, 0.35f, 0.62f, 0.72f, 0.28f, 0.75f, 0.55f, 2, 0.15f),
        cabOf (CabType::C4x10, MicType::Condenser, 0.20f, 0.22f, 0.15f, true),
        rigOf (0.0f, -4.0f, 0.25f),
        { ped (PedalType::Compressor, { -20.0f, 5.0f, 4.0f, 120.0f, 6.0f, 90.0f, 1.0f }),
          ped (PedalType::GraphicEq,  { 4.0f, 2.0f, -5.0f, -3.0f, 1.0f, 5.0f, 4.0f }) }
    });

    p.push_back ({
        "Octave Funk", "Funk",
        "Sub-octave under a filtered top end. Fat and synthetic without actually "
        "being a synth - the divider tracks fast enough to play sixteenths on.",
        "Single notes only above the fifth fret; octave dividers cannot track "
        "double stops. Keep the octave below the direct level.",
        ampOf (AmpType::AshdownMag, 0.40f, 0.65f, 0.62f, 0.50f, 0.60f, 0.40f, 2, 0.15f),
        cabOf (CabType::C2x10, MicType::DynamicLarge, 0.30f, 0.25f, 0.10f, true),
        rigOf (0.0f, -5.0f, 0.20f),
        { ped (PedalType::Octave,         { 85.0f, 65.0f, 0.0f, 0.0f, 1400.0f }),
          ped (PedalType::EnvelopeFilter, { 55.0f, 300.0f, 5.0f, 8.0f, 180.0f, 1.0f, 0.0f, 70.0f }) }
    });

    p.push_back ({
        "Fuzz Funk", "Funk",
        "Fuzz with the low end kept clean, under an envelope filter. The 1970s "
        "funk lead-bass sound - dirty, vocal and still solid underneath.",
        "The Blend control on the fuzz is how you dial in how much of this is "
        "actually distorted. Start at 70% and work from there.",
        ampOf (AmpType::AmpegSvt, 0.50f, 0.70f, 0.58f, 0.60f, 0.58f, 0.45f, 2, 0.45f),
        cabOf (CabType::C4x10, MicType::DynamicLarge, 0.28f, 0.25f, 0.12f, true),
        rigOf (0.0f, -5.0f, 0.15f),
        { ped (PedalType::Fuzz,           { 70.0f, 40.0f, -9.0f, 110.0f, 70.0f }),
          ped (PedalType::EnvelopeFilter, { 58.0f, 260.0f, 7.0f, 6.0f, 140.0f, 1.0f, 0.0f, 85.0f }) }
    });

    // ---- Reggae / dub -------------------------------------------------------
    p.push_back ({
        "Kingston Foundation", "Reggae / Dub",
        "Flip-top into a sealed 1x15, tone rolled off, everything above 1 kHz gone. "
        "The reggae bass sound is almost entirely the first two harmonics.",
        "Flatwounds, neck pickup, tone off, play over the fingerboard with the "
        "fleshy part of your fingers. Mute with your left hand - the space between "
        "notes matters more than the notes.",
        ampOf (AmpType::AmpegB15, 0.35f, 0.50f, 0.78f, 0.42f, 0.12f, 0.0f, 2, 0.7f),
        cabOf (CabType::C1x15, MicType::DynamicLarge, 0.55f, 0.15f, 0.08f),
        rigOf (0.0f, -3.0f, 0.10f),
        { ped (PedalType::Compressor, { -20.0f, 3.5f, 20.0f, 250.0f, 6.0f, 100.0f, 0.0f }),
          ped (PedalType::GraphicEq,  { 3.0f, 3.0f, -2.0f, -6.0f, -9.0f, -12.0f, -12.0f }) }
    });

    p.push_back ({
        "Dub Delay", "Reggae / Dub",
        "The same foundation tone with a tempo-synced analog delay in the amp's "
        "loop. Set the metronome and the repeats will lock to it.",
        "Use the delay sparingly and let it fall on the off-beat. The feedback is "
        "high enough to build - ride the mix control.",
        ampOf (AmpType::AmpegB15, 0.38f, 0.52f, 0.80f, 0.40f, 0.15f, 0.0f, 2, 0.7f),
        cabOf (CabType::C1x15, MicType::Ribbon, 0.50f, 0.30f, 0.20f),
        rigOf (0.0f, -4.0f, 0.10f),
        { ped (PedalType::Compressor, { -20.0f, 3.5f, 20.0f, 250.0f, 6.0f, 100.0f, 0.0f }),
          ped (PedalType::Delay, { 380.0f, 55.0f, 30.0f, 2200.0f, 1.0f, 1.0f, 6.0f }, PedalPlacement::AmpLoop),
          ped (PedalType::Reverb, { 55.0f, 70.0f, 25.0f, 260.0f, 22.0f }, PedalPlacement::PostCab) }
    });

    // ---- Jazz / fretless ----------------------------------------------------
    p.push_back ({
        "Upright Impression", "Jazz",
        "Dark, woody and short. A 1x15 with a ribbon mic well back, low gain, and "
        "the top end rolled off to imitate a double bass in a jazz trio.",
        "Play over the end of the fingerboard with a flat finger, and mute each "
        "note early. The decay is doing as much work as the tone.",
        ampOf (AmpType::AmpegB15, 0.30f, 0.48f, 0.62f, 0.52f, 0.20f, 0.0f, 2, 0.6f),
        cabOf (CabType::C1x15, MicType::Ribbon, 0.60f, 0.55f, 0.28f),
        rigOf (0.0f, -3.0f, 0.10f),
        { ped (PedalType::Compressor, { -24.0f, 2.5f, 25.0f, 300.0f, 6.0f, 80.0f, 0.0f }),
          ped (PedalType::GraphicEq,  { 2.0f, 1.0f, 0.0f, -3.0f, -7.0f, -10.0f, -10.0f }) }
    });

    p.push_back ({
        "Singing Fretless", "Jazz",
        "Bridge pickup, strong mids, light chorus. The fretless 'mwah' is a "
        "midrange phenomenon - it comes from the string hitting the fingerboard, "
        "so the EQ has to leave that range alone.",
        "Bridge pickup favoured, vibrato from the left hand, and play close to the "
        "bridge. Do not scoop the mids or the character disappears.",
        ampOf (AmpType::AshdownMag, 0.38f, 0.62f, 0.55f, 0.68f, 0.62f, 0.45f, 2, 0.15f),
        cabOf (CabType::C2x10, MicType::Condenser, 0.25f, 0.30f, 0.15f, true),
        rigOf (0.0f, -4.0f, 0.25f),
        { ped (PedalType::Compressor, { -20.0f, 3.0f, 15.0f, 180.0f, 5.0f, 70.0f, 0.0f }),
          ped (PedalType::Chorus, { 0.45f, 30.0f, 18.0f, 1.0f, 140.0f, 22.0f }, PedalPlacement::AmpLoop) }
    });

    p.push_back ({
        "Smooth Jazz DI", "Jazz",
        "Studio DI with a gentle hi-fi curve and firm compression. Clean, even and "
        "completely uncoloured - what a session player hands an engineer.",
        "Consistency is everything. If the compressor is working more than about "
        "4 dB, your right hand is uneven, not the setting.",
        ampOf (AmpType::StudioDI, 0.20f, 0.5f, 0.58f, 0.48f, 0.62f, 0.0f, 3, 0.0f),
        cabOf (CabType::Bypass, MicType::Condenser, 0.0f, 0.0f, 0.0f),
        rigOf (0.0f, -4.0f, 1.0f),
        { ped (PedalType::Compressor, { -24.0f, 3.0f, 12.0f, 180.0f, 7.0f, 100.0f, 0.0f }) }
    });

    // ---- Prog / fusion ------------------------------------------------------
    p.push_back ({
        "Prog Chorus", "Prog",
        "Clean, wide and articulate, with chorus in the amp loop and a short delay "
        "behind it. The low end stays dry so the chords keep their footing.",
        "Works best playing in the upper register. The chorus Low Keep control is "
        "what stops it turning to soup below the fifth fret.",
        ampOf (AmpType::AshdownMag, 0.35f, 0.62f, 0.60f, 0.58f, 0.65f, 0.45f, 2, 0.15f),
        cabOf (CabType::C4x10, MicType::Condenser, 0.28f, 0.30f, 0.18f, true),
        rigOf (0.0f, -4.0f, 0.25f),
        { ped (PedalType::Compressor, { -18.0f, 3.0f, 12.0f, 160.0f, 4.0f, 70.0f, 0.0f }),
          ped (PedalType::Chorus, { 0.35f, 45.0f, 20.0f, 2.0f, 150.0f, 35.0f }, PedalPlacement::AmpLoop),
          ped (PedalType::Delay,  { 420.0f, 25.0f, 18.0f, 3500.0f, 0.0f, 1.0f, 6.0f }, PedalPlacement::AmpLoop) }
    });

    p.push_back ({
        "Alt-Prog Grind", "Prog",
        "Distortion, chorus and a big cab. Aggressive but still tuned - the "
        "distortion keeps the fundamental clean and the chorus sits on the "
        "harmonics above it.",
        "Pick near the bridge. If the chords blur, raise Low Keep on both the "
        "distortion and the chorus.",
        ampOf (AmpType::AmpegSvt, 0.62f, 0.78f, 0.58f, 0.65f, 0.65f, 0.55f, 3, 0.4f),
        cabOf (CabType::C4x12, MicType::DynamicSmall, 0.20f, 0.22f, 0.12f),
        rigOf (1.0f, -5.0f, 0.15f),
        { ped (PedalType::Distortion, { 48.0f, 3.0f, -8.0f, 130.0f, 65.0f }),
          ped (PedalType::Chorus, { 0.6f, 40.0f, 14.0f, 1.0f, 180.0f, 28.0f }, PedalPlacement::AmpLoop) }
    });

    // ---- Pop / indie --------------------------------------------------------
    p.push_back ({
        "Pop Session", "Pop",
        "Clean, compressed, slightly bright, mostly DI with a little cab under it. "
        "The safe choice for a pop track: nobody will ever ask you to change it.",
        "Keep the DI blend around 40%. The cab is there for weight, the DI for "
        "definition.",
        ampOf (AmpType::StudioDI, 0.25f, 0.55f, 0.58f, 0.50f, 0.60f, 0.0f, 2, 0.0f),
        cabOf (CabType::C1x12, MicType::Condenser, 0.30f, 0.25f, 0.12f, true),
        rigOf (0.0f, -4.0f, 0.40f),
        { ped (PedalType::Compressor, { -20.0f, 4.0f, 10.0f, 160.0f, 6.0f, 100.0f, 0.0f }) }
    });

    p.push_back ({
        "Indie Melodic", "Pop",
        "Bright, picked, high-register melodic bass with chorus - the post-punk "
        "sound where the bass carries the hook and the guitar does the texture.",
        "Play above the twelfth fret with a pick, bridge pickup. Do not fight the "
        "thin low end; that is what makes room for it in the mix.",
        ampOf (AmpType::FenderBassman, 0.48f, 0.68f, 0.45f, 0.62f, 0.72f, 0.60f, 2, 0.5f),
        cabOf (CabType::C2x12, MicType::DynamicSmall, 0.22f, 0.28f, 0.18f),
        rigOf (0.0f, -4.0f, 0.15f),
        { ped (PedalType::Chorus, { 0.8f, 55.0f, 14.0f, 1.0f, 200.0f, 45.0f }),
          ped (PedalType::Reverb, { 40.0f, 60.0f, 12.0f, 280.0f, 15.0f }, PedalPlacement::PostCab) }
    });

    // ---- Synth / electronic -------------------------------------------------
    p.push_back ({
        "Analog Synth Bass", "Synth",
        "Monophonic synth voice with a resonant filter sweep, plus a sub octave. "
        "Tracks the string directly, so it plays like a bass rather than like a "
        "keyboard.",
        "Single notes, clean attack, no double stops. Play above the fifth fret - "
        "tracking below that gets unreliable on any pedal of this kind.",
        ampOf (AmpType::AshdownMag, 0.30f, 0.60f, 0.65f, 0.48f, 0.60f, 0.35f, 2, 0.1f),
        cabOf (CabType::C2x12, MicType::Condenser, 0.30f, 0.25f, 0.10f),
        rigOf (0.0f, -6.0f, 0.30f),
        { ped (PedalType::Synth, { 60.0f, 420.0f, 6.5f, 70.0f, 200.0f, 1.0f, 45.0f, 20.0f }) }
    });

    p.push_back ({
        "Sub Drop", "Synth",
        "Sub-harmoniser on the amp plus an octave pedal, into a big ported cab. "
        "For the parts where the bass is meant to be felt rather than heard.",
        "Check this on a system that actually reproduces 40 Hz before you trust "
        "it. On small speakers it will sound like nothing is happening.",
        [] { AmpSettings a = ampOf (AmpType::AshdownMag, 0.30f, 0.65f, 0.72f, 0.42f, 0.50f, 0.25f, 1, 0.1f);
             a.subLevel = 0.5f; return a; }(),
        cabOf (CabType::C4x10, MicType::DynamicLarge, 0.40f, 0.20f, 0.08f),
        rigOf (0.0f, -6.0f, 0.25f),
        { ped (PedalType::Octave,     { 90.0f, 75.0f, 25.0f, 0.0f, 900.0f }),
          ped (PedalType::Compressor, { -22.0f, 6.0f, 8.0f, 140.0f, 6.0f, 100.0f, 2.0f }) }
    });

    p.push_back ({
        "Broken Circuit", "Synth",
        "Bit crusher and ring modulator into a distorted amp. Deliberately "
        "unmusical - a texture for an intro or a breakdown, not a part you play "
        "through a whole song.",
        "Set the ring modulator frequency by ear against the key of the track; "
        "it is inharmonic, so nothing about it is automatic.",
        ampOf (AmpType::DriverDI, 0.65f, 0.60f, 0.55f, 0.60f, 0.60f, 0.50f, 2, 0.0f),
        cabOf (CabType::C2x10, MicType::DynamicSmall, 0.20f, 0.20f, 0.10f),
        rigOf (0.0f, -7.0f, 0.20f),
        { ped (PedalType::BitCrusher, { 6.0f, 8.0f, 55.0f }),
          ped (PedalType::RingMod,    { 145.0f, 0.0f, 30.0f }),
          ped (PedalType::Distortion, { 45.0f, 2.0f, -10.0f, 90.0f, 70.0f }) }
    });

    // ---- Studio / utility ---------------------------------------------------
    p.push_back ({
        "Driver DI Grit", "Studio",
        "Bass driver preamp straight to the desk: blend a clean low end with a "
        "driven upper band and a built-in speaker rolloff. The tone on an enormous "
        "number of records, precisely because it needs no amp.",
        "The Blend control is the whole pedal. Below halfway it is a clean DI with "
        "attitude; above it, it is a distortion box.",
        [] { AmpSettings a = ampOf (AmpType::DriverDI, 0.55f, 0.55f, 0.62f, 0.45f, 0.65f, 0.45f, 2, 0.0f);
             a.blend = 0.65f; return a; }(),
        cabOf (CabType::Bypass, MicType::Condenser, 0.0f, 0.0f, 0.0f),
        rigOf (0.0f, -4.0f, 1.0f),
        { ped (PedalType::Compressor, { -18.0f, 3.5f, 12.0f, 180.0f, 4.0f, 80.0f, 0.0f }) }
    });

    p.push_back ({
        "Flat Reference", "Studio",
        "Everything neutral: no pedals, clean DI, no cabinet. Use this to check "
        "your instrument, set your input gain, and hear what you are actually "
        "playing before any of the rest of it gets in the way.",
        "Set input gain so peaks land around -12 dBFS on the input meter. Every "
        "other preset assumes you did that.",
        ampOf (AmpType::StudioDI, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 2, 0.0f),
        cabOf (CabType::Bypass, MicType::Condenser, 0.0f, 0.0f, 0.0f),
        rigOf (0.0f, -3.0f, 1.0f),
        { }
    });

    p.push_back ({
        "Practice Room", "Studio",
        "A moderate, forgiving amp sound for playing along with tracks: enough "
        "compression to stay even, enough midrange to hear yourself, and nothing "
        "that will flatter a sloppy right hand.",
        "This is the one to leave the metronome running under. It deliberately "
        "does not hide timing or note-length problems.",
        ampOf (AmpType::AshdownMag, 0.40f, 0.60f, 0.58f, 0.58f, 0.58f, 0.35f, 2, 0.2f),
        cabOf (CabType::C2x10, MicType::DynamicLarge, 0.30f, 0.25f, 0.15f, true),
        rigOf (0.0f, -4.0f, 0.25f),
        { ped (PedalType::Compressor, { -18.0f, 3.0f, 15.0f, 200.0f, 4.0f, 70.0f, 2.0f }) }
    });

    p.push_back ({
        "Country Flats", "Country",
        "Muted, even and out of the way. Flatwounds, palm muting, and an EQ that "
        "leaves the whole midrange to the vocal and the telecaster.",
        "Palm mute everything. The part is meant to be felt, not noticed.",
        ampOf (AmpType::FenderBassman, 0.35f, 0.58f, 0.60f, 0.48f, 0.42f, 0.25f, 2, 0.55f),
        cabOf (CabType::C1x15, MicType::DynamicLarge, 0.40f, 0.30f, 0.15f),
        rigOf (0.0f, -3.0f, 0.20f),
        { ped (PedalType::Compressor, { -20.0f, 4.0f, 18.0f, 220.0f, 6.0f, 100.0f, 0.0f }) }
    });

    p.push_back ({
        "Blues Breakup", "Blues",
        "Valve amp just past the point of breakup, 2x12, ribbon mic. Cleans up "
        "when you back off and growls when you dig in - the amp is the effect.",
        "Play with your fingers and use your right hand as the gain control. Set "
        "the amp so that medium playing is just clean.",
        ampOf (AmpType::FenderBassman, 0.68f, 0.72f, 0.58f, 0.52f, 0.55f, 0.45f, 2, 0.65f),
        cabOf (CabType::C2x12, MicType::Ribbon, 0.32f, 0.38f, 0.22f),
        rigOf (0.0f, -4.0f, 0.10f),
        { }
    });

    p.push_back ({
        "Gospel Chops", "Gospel",
        "Modern gospel: hi-fi, compressed, bright top, tight low end, and enough "
        "clarity for fast passages and double stops to stay legible.",
        "Compression is set fast and firm so sixteenth-note runs stay even. Roll "
        "the 10 kHz band back if the string noise becomes distracting.",
        ampOf (AmpType::AshdownMag, 0.32f, 0.62f, 0.62f, 0.52f, 0.70f, 0.50f, 3, 0.1f),
        cabOf (CabType::C4x10, MicType::Condenser, 0.22f, 0.25f, 0.14f, true),
        rigOf (0.0f, -4.0f, 0.35f),
        { ped (PedalType::Compressor, { -22.0f, 5.0f, 5.0f, 120.0f, 7.0f, 95.0f, 1.0f }),
          ped (PedalType::GraphicEq,  { 2.0f, 1.0f, -2.0f, 0.0f, 2.0f, 3.0f, 2.0f }) }
    });

    p.push_back ({
        "Hip-Hop Live", "Hip-Hop",
        "Playing live over programmed drums: sub-heavy, very controlled, and "
        "everything above 2 kHz removed so it never fights the sample.",
        "Short notes, no sustain, no string noise. Mute hard with both hands.",
        ampOf (AmpType::StudioDI, 0.22f, 0.55f, 0.72f, 0.42f, 0.35f, 0.0f, 1, 0.0f),
        cabOf (CabType::C1x15, MicType::DynamicLarge, 0.45f, 0.18f, 0.06f),
        rigOf (0.0f, -4.0f, 0.55f),
        { ped (PedalType::Compressor, { -24.0f, 6.0f, 6.0f, 130.0f, 8.0f, 100.0f, 2.0f }),
          ped (PedalType::GraphicEq,  { 4.0f, 2.0f, -1.0f, -4.0f, -8.0f, -12.0f, -12.0f }) }
    });

    return p;
}

} // namespace

static const std::vector<Preset>& presets()
{
    static const std::vector<Preset> table = buildPresets();
    return table;
}

int numPresets() { return (int) presets().size(); }

const Preset& getPreset (int index)
{
    const auto& table = presets();
    return table[(size_t) std::max (0, std::min (index, (int) table.size() - 1))];
}

int findPreset (const std::string& name)
{
    const auto& table = presets();
    for (int i = 0; i < (int) table.size(); ++i) if (table[(size_t) i].name == name) return i;
    return -1;
}

std::vector<std::string> presetCategories()
{
    std::vector<std::string> out;
    for (const auto& p : presets())
    {
        bool found = false;
        for (const auto& c : out) if (c == p.category) { found = true; break; }
        if (! found) out.push_back (p.category);
    }
    return out;
}

void applyPreset (Rig& rig, const Preset& preset)
{
    rig.amp().setSettings (preset.amp);

    RigSettings r = preset.rig;
    r.oversampling = rig.getSettings().oversampling;   // a performance setting, not a tone one
    r.tunerActive = rig.getSettings().tunerActive;
    r.muteWhileTuning = rig.getSettings().muteWhileTuning;
    rig.setSettings (r);

    rig.requestCabinetSettings (preset.cab);

    auto& board = rig.pedalboard();
    board.restoreDefaultOrder();

    for (int i = 0; i < Pedalboard::kNumSlots; ++i)
    {
        Pedal* p = board.pedalOfType ((PedalType) i);
        if (p == nullptr) continue;
        p->setEnabled (false);
        p->restoreDefaults();
        board.setPlacement ((PedalType) i, PedalPlacement::FrontOfAmp);
    }

    for (const auto& pp : preset.pedals)
    {
        Pedal* p = board.pedalOfType (pp.type);
        if (p == nullptr) continue;
        board.setPlacement (pp.type, pp.placement);
        for (int q = 0; q < (int) pp.params.size() && q < p->numParams(); ++q)
            p->setParamValue (q, pp.params[(size_t) q]);
        p->setEnabled (true);
    }

    rig.serviceMessageThread();
}

} // namespace bassamp
