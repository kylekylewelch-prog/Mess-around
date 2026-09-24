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

    // ---- Signature rigs -----------------------------------------------------
    // Built from documented setups. The name describes the sound; the reference
    // line says whose rig it came from and which record to check it against.

    p.push_back ({
        "Clank & Growl", "Signature",
        "Bridge-pickup Rickenbacker into a valve 300 W head and an 8x10, Ultra Hi "
        "engaged, midrange selector parked at 1.6 kHz. Geddy Lee's Moving Pictures "
        "rig - Tom Sawyer, YYZ, Limelight.",
        "This is a midrange sound with a low end attached, not the other way round. "
        "Play hard over the neck pickup with high action and let the attack clank. "
        "If you reach for the bass control you have lost it.",
        []{ auto a = ampOf (AmpType::AmpegSvt, 0.72f, 0.78f, 0.42f, 0.78f, 0.72f, 0.62f, 3, 0.40f);
             a.ultraHi = true; return a; }(),
        cabOf (CabType::C8x10, MicType::DynamicSmall, 0.18f, 0.18f, 0.10f),
        rigOf (1.0f, -4.0f, 0.12f),
        { ped (PedalType::Compressor, { -14.0f, 2.5f, 10.0f, 180.0f, 3.0f, 55.0f, 1.0f }),
          ped (PedalType::GraphicEq,  { -1.0f, -2.0f, 1.0f, 4.0f, 3.5f, 1.5f, 0.0f }) }
    });

    p.push_back ({
        "Eighties Hi-Fi", "Signature",
        "Active multi-laminate bass into a solid-state head and a horn-loaded 4x10, "
        "condenser mic. The Wal-and-rack era - Power Windows, Hold Your Fire. Tight, "
        "scooped, fast, with the attack doing the work the distortion used to.",
        "Much more controlled than the Rickenbacker sound. Lighter right hand, and "
        "let the chorus do the width. Turn the chorus off and it is a very usable "
        "modern rock tone.",
        ampOf (AmpType::AshdownMag, 0.35f, 0.65f, 0.62f, 0.42f, 0.68f, 0.55f, 2, 0.15f),
        cabOf (CabType::C4x10, MicType::Condenser, 0.25f, 0.28f, 0.14f, true),
        rigOf (0.0f, -4.0f, 0.25f),
        { ped (PedalType::Compressor, { -20.0f, 3.5f, 14.0f, 200.0f, 5.0f, 85.0f, 0.0f }),
          ped (PedalType::Chorus,     { 0.45f, 28.0f, 14.0f, 1.0f, 140.0f, 25.0f }),
          ped (PedalType::GraphicEq,  { 1.0f, 2.0f, -3.0f, -1.0f, 2.0f, 2.0f, 1.0f }) }
    });

    p.push_back ({
        "Attitude Split", "Signature",
        "The bi-amped split rig: neck pickup clean into the bass amp, bridge pickup "
        "into a distorted guitar amp. Billy Sheehan's Attitude setup. The Low Keep "
        "control on the distortion is doing exactly what his crossover did.",
        "Low Keep at 250 Hz is the whole trick - everything under it stays clean and "
        "the grind only lives on top. Push it lower for more dirt, higher to clean "
        "up the bottom. Three-finger right hand, and the bends are part of it.",
        ampOf (AmpType::AmpegSvt, 0.50f, 0.70f, 0.60f, 0.60f, 0.62f, 0.50f, 2, 0.45f),
        cabOf (CabType::C8x10, MicType::DynamicLarge, 0.28f, 0.22f, 0.10f),
        rigOf (1.0f, -4.0f, 0.25f),
        { ped (PedalType::Compressor, { -16.0f, 3.0f, 12.0f, 190.0f, 4.0f, 70.0f, 1.0f }),
          ped (PedalType::Distortion, { 70.0f, 3.0f, -5.0f, 250.0f, 100.0f }) }
    });

    p.push_back ({
        "Three-Finger Lead", "Signature",
        "The same split pushed much harder, for lead-bass playing - Shy Boy, Addicted "
        "to That Rush. Tapping and fast runs need the upper mids to stay legible "
        "when the distortion is this heavy.",
        "The 1.6 kHz band is what makes tapped notes speak. If fast passages turn to "
        "mush, raise Low Keep rather than backing off the drive.",
        ampOf (AmpType::AmpegSvt, 0.72f, 0.80f, 0.55f, 0.72f, 0.70f, 0.62f, 3, 0.40f),
        cabOf (CabType::C4x10, MicType::DynamicSmall, 0.20f, 0.20f, 0.10f, true),
        rigOf (2.0f, -5.0f, 0.20f),
        { ped (PedalType::Compressor, { -18.0f, 4.0f, 5.0f, 140.0f, 5.0f, 80.0f, 1.0f }),
          ped (PedalType::Distortion, { 82.0f, 5.0f, -7.0f, 200.0f, 100.0f }),
          ped (PedalType::GraphicEq,  { 0.0f, -1.0f, 1.0f, 3.0f, 4.0f, 2.0f, 0.0f }) }
    });

    p.push_back ({
        "Folded Horn", "Signature",
        "Jazz Bass into a solid-state head and an 18-inch folded horn cabinet - John "
        "Paul Jones's Acoustic 360. Deep, round, and with a low-mid honk that is the "
        "reason the bass is so audible on Led Zeppelin II.",
        "The Lemon Song and Ramble On. Fingerstyle over the neck pickup, roundwounds, "
        "and play melodically - this tone was built to be a second voice, not a floor. "
        "The 350 Hz midrange setting is the honk; move it and the character goes.",
        ampOf (AmpType::AshdownMag, 0.45f, 0.62f, 0.70f, 0.58f, 0.42f, 0.25f, 1, 0.20f),
        cabOf (CabType::C1x15, MicType::DynamicLarge, 0.40f, 0.30f, 0.16f),
        rigOf (0.0f, -3.0f, 0.18f),
        { ped (PedalType::Compressor, { -18.0f, 2.5f, 20.0f, 240.0f, 4.0f, 75.0f, 0.0f }),
          ped (PedalType::GraphicEq,  { 1.0f, 2.0f, 2.5f, 0.0f, -1.0f, -2.0f, -3.0f }) }
    });

    p.push_back ({
        "Alembic Hi-Fi", "Signature",
        "Active through-neck bass with low-pass filter electronics into a bright "
        "horn-loaded rig. The Presence-era sound - Achilles Last Stand. Extended at "
        "both ends, fast, and far more modern than anything else on those records.",
        "Built for sixteenth-note stamina at speed. Very little compression on "
        "purpose: the dynamics are the performance. Fingers close to the bridge.",
        ampOf (AmpType::AshdownMag, 0.30f, 0.66f, 0.60f, 0.48f, 0.66f, 0.50f, 3, 0.15f),
        cabOf (CabType::C4x10, MicType::Condenser, 0.22f, 0.26f, 0.12f, true),
        rigOf (0.0f, -4.0f, 0.30f),
        { ped (PedalType::Compressor, { -22.0f, 2.0f, 18.0f, 200.0f, 3.0f, 50.0f, 0.0f }),
          ped (PedalType::GraphicEq,  { 2.0f, 1.0f, -2.0f, 0.0f, 2.0f, 2.5f, 2.0f }) }
    });

    p.push_back ({
        "Bassically", "Signature",
        "Fuzz into a wah into a pushed valve head and a 4x12 - the unaccompanied bass "
        "intro to N.I.B. Geezer Butler's 1970 rig was Laney heads and guitar cabs, "
        "which is why it sounds more like a guitar than a bass amp.",
        "Assign the Wah's Pedal control to an expression pedal, or switch Wah Mode to "
        "Auto LFO and set the rate by ear. Low Keep at 110 Hz keeps the fundamental "
        "under the fuzz so the line still reads as a bass.",
        ampOf (AmpType::FenderBassman, 0.72f, 0.78f, 0.55f, 0.62f, 0.55f, 0.45f, 2, 0.62f),
        cabOf (CabType::C4x12, MicType::DynamicSmall, 0.25f, 0.28f, 0.16f),
        rigOf (1.0f, -5.0f, 0.08f),
        { ped (PedalType::Fuzz, { 78.0f, 55.0f, -6.0f, 110.0f, 100.0f }),
          ped (PedalType::Wah,  { 50.0f, 280.0f, 1800.0f, 5.5f, 0.0f, 1.2f, 100.0f }) }
    });

    p.push_back ({
        "Birmingham Riff", "Signature",
        "The same amp without the wah, for the body of the song: bass doubling the "
        "guitar riff an octave down with enough grit to be heard as a separate part. "
        "Early Sabbath, where the bass is never just holding roots.",
        "Fingerstyle, dig in, and follow the guitar but fill the gaps - that is the "
        "whole Geezer approach. Back the Fuzz Blend off if the riff needs to be "
        "cleaner under a vocal.",
        ampOf (AmpType::FenderBassman, 0.65f, 0.76f, 0.58f, 0.66f, 0.50f, 0.40f, 2, 0.60f),
        cabOf (CabType::C4x12, MicType::Ribbon, 0.28f, 0.30f, 0.15f),
        rigOf (0.0f, -4.0f, 0.12f),
        { ped (PedalType::Fuzz,      { 55.0f, 45.0f, -8.0f, 140.0f, 70.0f }),
          ped (PedalType::GraphicEq, { 0.0f, 1.0f, 2.0f, 2.0f, 1.0f, -1.0f, -3.0f }) }
    });

    p.push_back ({
        "Snakepit Direct", "Signature",
        "The Motown console chain rather than the amp: DI dominant, valve limiting "
        "doing most of the tone shaping, flatwounds and a foam mute. What's Going On, "
        "For Once in My Life, Bernadette.",
        "Heavier limiting than feels right when you solo it - that is the record. One "
        "finger, over the end of the fingerboard, and let the compressor even out the "
        "line rather than your right hand.",
        ampOf (AmpType::AmpegB15, 0.35f, 0.50f, 0.64f, 0.48f, 0.18f, 0.0f, 2, 0.70f),
        cabOf (CabType::C1x15, MicType::DynamicLarge, 0.48f, 0.18f, 0.06f),
        rigOf (0.0f, -3.0f, 0.55f),
        { ped (PedalType::Compressor, { -26.0f, 6.0f, 15.0f, 200.0f, 9.0f, 100.0f, 0.0f }),
          ped (PedalType::GraphicEq,  { 2.0f, 1.5f, 0.0f, -2.0f, -5.0f, -9.0f, -12.0f }) }
    });

    p.push_back ({
        "Hook Melodic", "Signature",
        "The same flatwound-and-foam voice with the mids opened back up, so busy "
        "sixteenth-note lines stay legible on a stage instead of in a control room. "
        "What you actually need to play those parts live.",
        "Keep the treble down - the definition here comes from 400 to 800 Hz, not "
        "from top end. If the line disappears in the band, push 800 Hz before you "
        "touch anything else.",
        ampOf (AmpType::AmpegB15, 0.45f, 0.58f, 0.58f, 0.62f, 0.26f, 0.0f, 2, 0.68f),
        cabOf (CabType::C1x15, MicType::DynamicLarge, 0.38f, 0.24f, 0.12f),
        rigOf (0.0f, -3.0f, 0.30f),
        { ped (PedalType::Compressor, { -20.0f, 4.0f, 18.0f, 220.0f, 6.0f, 90.0f, 0.0f }),
          ped (PedalType::GraphicEq,  { 1.0f, 1.0f, 2.5f, 2.0f, -2.0f, -6.0f, -9.0f }) }
    });

    p.push_back ({
        "Stingray Grind", "Signature",
        "Active humbucker bass into a valve head pushed hard, with parallel "
        "distortion so the low end survives. Tim Commerford's riff tone - Bombtrack, "
        "Bulls on Parade, the Killing in the Name breakdown.",
        "Distortion Mix at 70% is the important number: the dry path keeps the "
        "fundamental while the wet path does the grind. Hard fingerstyle up near the "
        "neck, and mute everything you are not playing.",
        ampOf (AmpType::AmpegSvt, 0.68f, 0.80f, 0.58f, 0.70f, 0.66f, 0.58f, 2, 0.40f),
        cabOf (CabType::C8x10, MicType::DynamicSmall, 0.22f, 0.20f, 0.10f),
        rigOf (1.0f, -4.0f, 0.15f),
        { ped (PedalType::Compressor, { -16.0f, 4.0f, 6.0f, 150.0f, 4.0f, 65.0f, 1.0f }),
          ped (PedalType::Distortion, { 55.0f, 4.0f, -6.0f, 140.0f, 70.0f }),
          ped (PedalType::GraphicEq,  { 0.0f, 1.0f, 1.0f, 3.0f, 3.0f, 1.0f, 0.0f }) }
    });

    p.push_back ({
        "Rap-Rock Snap", "Signature",
        "The clean side of the same bass: Ultra Lo engaged for the scoop, fast "
        "compression for ghost notes, and enough top for the attack to read as "
        "percussion. Take the Power Back.",
        "The sixteenth-note ghost notes are the part. Set the compressor fast and "
        "let it even them out, then play the accents harder than feels necessary.",
        []{ auto a = ampOf (AmpType::AmpegSvt, 0.42f, 0.72f, 0.66f, 0.42f, 0.70f, 0.55f, 2, 0.35f);
             a.ultraLo = true; return a; }(),
        cabOf (CabType::C4x10, MicType::DynamicSmall, 0.20f, 0.22f, 0.10f, true),
        rigOf (0.0f, -4.0f, 0.20f),
        { ped (PedalType::Compressor, { -20.0f, 5.0f, 3.0f, 110.0f, 6.0f, 90.0f, 1.0f }),
          ped (PedalType::GraphicEq,  { 3.0f, 3.0f, -3.0f, 0.0f, 3.0f, 3.0f, 1.0f }) }
    });

    p.push_back ({
        "Thumb Bark", "Signature",
        "Wenge-and-bubinga neck-through bass into a valve head and a horn-loaded "
        "4x10. Ryan Martinie's Warwick Thumb tone - Dig, Determined. The bark lives "
        "between 800 Hz and 1.6 kHz and it is the whole point of the instrument.",
        "Slap, pop, tap and chord all need the same thing here: fast compression and "
        "the upper mids left alone. Play over the end of the fingerboard for the "
        "woody note and up at the bridge for the snap.",
        ampOf (AmpType::AmpegSvt, 0.60f, 0.75f, 0.55f, 0.72f, 0.70f, 0.60f, 3, 0.35f),
        cabOf (CabType::C4x10, MicType::DynamicSmall, 0.18f, 0.20f, 0.10f, true),
        rigOf (0.0f, -4.0f, 0.20f),
        { ped (PedalType::Compressor, { -20.0f, 5.0f, 3.0f, 120.0f, 6.0f, 85.0f, 1.0f }),
          ped (PedalType::Overdrive,  { 32.0f, 60.0f, -5.0f, 55.0f }),
          ped (PedalType::GraphicEq,  { 1.0f, 0.0f, 1.0f, 4.0f, 4.0f, 3.0f, 1.0f }) }
    });

    p.push_back ({
        "Melodic Over Drop", "Signature",
        "The clean counterpart: chorused, bright and sustained, for melodic bass "
        "carrying the song over down-tuned guitars. World So Cold, Death Blooms.",
        "The chorus keeps the low end dry at 130 Hz so the bottom stays solid while "
        "the harmonics move. A little reverb is deliberate - this part is a lead "
        "line, not a rhythm part.",
        ampOf (AmpType::AshdownMag, 0.28f, 0.62f, 0.60f, 0.52f, 0.68f, 0.52f, 3, 0.15f),
        cabOf (CabType::C4x10, MicType::Condenser, 0.24f, 0.28f, 0.16f, true),
        rigOf (0.0f, -4.0f, 0.28f),
        { ped (PedalType::Compressor, { -22.0f, 3.5f, 12.0f, 200.0f, 5.0f, 85.0f, 0.0f }),
          ped (PedalType::Chorus,     { 0.35f, 45.0f, 18.0f, 2.0f, 130.0f, 35.0f }),
          ped (PedalType::Reverb,     { 40.0f, 60.0f, 20.0f, 260.0f, 14.0f }, PedalPlacement::PostCab) }
    });

    // ---- Latin rock ---------------------------------------------------------
    p.push_back ({
        "Tumbao", "Latin Rock",
        "Precision with a flip-top valve head into a 1x15, mic close and off centre. "
        "The Abraxas-era Santana rhythm section sound: the bass has to state the "
        "fundamental clearly enough that the tumbao is felt, not just heard.",
        "Oye Como Va, Black Magic Woman. The anticipated notes - the and of two, and "
        "beat four - carry the groove, so give them the length and let the rest be "
        "short. Congas and timbales own everything above 2 kHz; stay out of it.",
        ampOf (AmpType::AmpegB15, 0.40f, 0.58f, 0.62f, 0.55f, 0.32f, 0.0f, 2, 0.65f),
        cabOf (CabType::C1x15, MicType::DynamicLarge, 0.40f, 0.25f, 0.14f),
        rigOf (0.0f, -3.0f, 0.18f),
        { ped (PedalType::Compressor, { -18.0f, 3.5f, 16.0f, 200.0f, 5.0f, 85.0f, 0.0f }),
          ped (PedalType::GraphicEq,  { 1.0f, 2.0f, 2.0f, 0.0f, -2.0f, -5.0f, -7.0f }) }
    });

    p.push_back ({
        "Barrio Groove", "Latin Rock",
        "Punchier and brighter, with the amp close enough to breakup that hard notes "
        "push it over. Latin funk and Chicano rock - Low Rider, El Chicano, the "
        "heavier end of the Santana catalogue.",
        "More mids and more attack than Tumbao, because here the bass is a hook "
        "rather than a foundation. Fingers near the bridge, notes short and even.",
        ampOf (AmpType::FenderBassman, 0.55f, 0.68f, 0.58f, 0.58f, 0.48f, 0.35f, 2, 0.55f),
        cabOf (CabType::C2x12, MicType::DynamicLarge, 0.32f, 0.28f, 0.16f),
        rigOf (0.0f, -4.0f, 0.15f),
        { ped (PedalType::Compressor, { -16.0f, 4.0f, 8.0f, 160.0f, 5.0f, 90.0f, 1.0f }),
          ped (PedalType::GraphicEq,  { 1.0f, 1.0f, 2.5f, 2.0f, 0.0f, -2.0f, -4.0f }) }
    });

    // ---- Classic rock -------------------------------------------------------
    p.push_back ({
        "Typewriter", "Classic Rock",
        "Fresh roundwounds, almost no compression and a deliberately bright rig - the "
        "sound that made bass a lead instrument in the late sixties. John Entwistle's "
        "bi-amped setup, where the highs went to guitar amps.",
        "My Generation, The Real Me. The clank is the sound: play hard with the "
        "fingers, let the strings hit the frets, and do not compress it away. If it "
        "feels too bright soloed it is probably right in the band.",
        []{ auto a = ampOf (AmpType::AmpegSvt, 0.40f, 0.72f, 0.52f, 0.62f, 0.80f, 0.70f, 3, 0.30f);
             a.ultraHi = true; return a; }(),
        cabOf (CabType::C4x10, MicType::Condenser, 0.15f, 0.18f, 0.10f, true),
        rigOf (0.0f, -4.0f, 0.25f),
        { ped (PedalType::Compressor, { -12.0f, 2.0f, 25.0f, 220.0f, 2.0f, 35.0f, 0.0f }),
          ped (PedalType::GraphicEq,  { 0.0f, -1.0f, 0.0f, 2.0f, 4.0f, 4.0f, 3.0f }) }
    });

    p.push_back ({
        "Seventies Session", "Classic Rock",
        "Precision with worn strings into a valve head backed off to where it stays "
        "clean but not clinical, ribbon mic well off the grille. The default studio "
        "bass sound of the decade.",
        "Warm without being dull: the treble is down but the mids are not. This is a "
        "supporting tone - root-and-fifth work, eighth notes, staying out of the way "
        "of everything else on the track.",
        ampOf (AmpType::AmpegSvt, 0.45f, 0.62f, 0.62f, 0.55f, 0.35f, 0.20f, 1, 0.65f),
        cabOf (CabType::C8x10, MicType::Ribbon, 0.40f, 0.42f, 0.20f),
        rigOf (0.0f, -3.0f, 0.15f),
        { ped (PedalType::Compressor, { -18.0f, 3.0f, 20.0f, 240.0f, 5.0f, 85.0f, 0.0f }) }
    });

    p.push_back ({
        "Twin Guitar Grit", "Classic Rock",
        "Midrange-forward and lightly overdriven, for sitting underneath harmonised "
        "guitars without disappearing. The Thin Lizzy problem: two guitars already own "
        "the mids, so the bass has to claim a narrow band and hold it.",
        "Pick or fingers both work. The overdrive is parallel at 55% - enough edge to "
        "be heard, not enough to blur the note. Push 400 Hz, not 50 Hz, if you get "
        "buried.",
        ampOf (AmpType::AmpegSvt, 0.60f, 0.74f, 0.52f, 0.68f, 0.58f, 0.45f, 2, 0.50f),
        cabOf (CabType::C4x12, MicType::DynamicLarge, 0.28f, 0.26f, 0.14f),
        rigOf (0.0f, -4.0f, 0.12f),
        { ped (PedalType::Compressor, { -16.0f, 3.0f, 12.0f, 180.0f, 4.0f, 70.0f, 1.0f }),
          ped (PedalType::Overdrive,  { 38.0f, 55.0f, -4.0f, 55.0f }) }
    });

    // ---- Emo / alt ----------------------------------------------------------
    p.push_back ({
        "Basement Pick", "Emo / Alt",
        "Pick-played Precision into a valve head with a little overdrive, small "
        "dynamic mic close to the cone. The midwest emo and pop-punk default - "
        "Jimmy Eat World, The Get Up Kids, early Foo Fighters.",
        "The pick click is a feature, not a problem: it is what keeps the bass "
        "audible when two overdriven guitars are playing the same chords. Pick "
        "halfway between the neck and bridge, downstrokes, and keep it even.",
        ampOf (AmpType::AmpegSvt, 0.62f, 0.72f, 0.52f, 0.66f, 0.68f, 0.55f, 2, 0.45f),
        cabOf (CabType::C4x10, MicType::DynamicSmall, 0.20f, 0.20f, 0.10f, true),
        rigOf (1.0f, -4.0f, 0.15f),
        { ped (PedalType::Compressor, { -16.0f, 3.5f, 8.0f, 160.0f, 4.0f, 70.0f, 1.0f }),
          ped (PedalType::Overdrive,  { 35.0f, 58.0f, -4.0f, 60.0f }) }
    });

    p.push_back ({
        "Midwest Clean", "Emo / Alt",
        "The other half of the same band: clean, chorused and bright, for the "
        "arpeggiated verses where the bass plays a countermelody rather than roots. "
        "American Football, Sunny Day Real Estate.",
        "Fingerstyle here, not a pick. The chorus keeps the bottom dry so the low "
        "notes stay solid under the shimmer. Let notes ring into each other.",
        ampOf (AmpType::AshdownMag, 0.28f, 0.60f, 0.58f, 0.52f, 0.66f, 0.45f, 2, 0.20f),
        cabOf (CabType::C2x10, MicType::Condenser, 0.26f, 0.30f, 0.18f),
        rigOf (0.0f, -4.0f, 0.30f),
        { ped (PedalType::Compressor, { -20.0f, 3.0f, 15.0f, 200.0f, 5.0f, 80.0f, 0.0f }),
          ped (PedalType::Chorus,     { 0.55f, 40.0f, 16.0f, 1.0f, 120.0f, 32.0f }),
          ped (PedalType::Reverb,     { 35.0f, 65.0f, 18.0f, 280.0f, 12.0f }, PedalPlacement::PostCab) }
    });

    p.push_back ({
        "Post-Hardcore Drive", "Emo / Alt",
        "Heavier and more saturated, with the low end split out of the dirt so the "
        "drop still lands. The louder end of the genre - Thursday, Brand New, Taking "
        "Back Sunday.",
        "Pick, hard, near the bridge. Low Keep at 120 Hz keeps the bottom clean under "
        "a lot of distortion; drop it to 80 Hz if you want the whole note to break up.",
        ampOf (AmpType::AmpegSvt, 0.72f, 0.78f, 0.55f, 0.70f, 0.66f, 0.58f, 2, 0.42f),
        cabOf (CabType::C4x12, MicType::DynamicSmall, 0.22f, 0.22f, 0.12f),
        rigOf (1.0f, -5.0f, 0.15f),
        { ped (PedalType::Compressor, { -18.0f, 4.0f, 6.0f, 150.0f, 5.0f, 75.0f, 1.0f }),
          ped (PedalType::Distortion, { 62.0f, 3.0f, -6.0f, 120.0f, 85.0f }) }
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
