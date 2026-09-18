# Rig research: where the presets come from

This is the reference material behind the factory presets. Each entry describes a
documented setup, what the important part of the signal chain actually was, and
which preset in the plugin is built from it.

A caveat worth stating up front: most of these players changed rigs constantly,
and studio setups rarely matched live ones. Where accounts conflict I have said
so rather than picking the version that makes a tidier story. Treat gear lists as
approximations and the *tonal description* as the thing to aim at, because that is
what the preset is actually modelling.

---

## Soul, Motown and R&B

**The Motown studio sound (James Jamerson, Detroit, 1960s).** A 1962 Fender
Precision with La Bella flatwounds that were famously never changed, a foam mute
under the bridge cover, and an Ampeg B-15 flip-top. Most Motown sessions were
recorded with both a DI and the amp, and the DI often dominates. The tone is
almost entirely fundamental and second harmonic: there is very little above
1 kHz, the notes are short, and the attack is soft because he played with one
finger over the end of the fingerboard.

→ **Detroit Thumb**. B-15 model into a 1x15, treble right down, opto compression,
DI blended in at 15%.

**Stax / Memphis (Duck Dunn).** Same basic formula, more grit. A Precision with
flats into a B-15 driven harder, so the amp is at the edge of breakup and the
player's dynamics push it over. Sitting behind the beat is as much of the sound as
the gear.

→ **Memphis Pocket**.

**Modern R&B and neo-soul.** Overwhelmingly a DI track, compressed firmly, with
the low end intact and a small presence lift for definition. There is often no
amp involved at all.

→ **Modern R&B DI**.

---

## Rock

**The SVT and 8x10.** The Ampeg SVT (1969 onwards) with the matching sealed 8x10
is the most recorded bass rig in rock. What it gives you is midrange authority: it
does not have the deepest low end of any cab, and that is the reason it works in a
loud band. Two valve gain stages into a 300 W push-pull output section means it
thickens rather than fizzes when pushed, and the supply sags under load, which is
why players describe it as "feeling" different from a solid-state head at the same
volume.

→ **Fridge Standard** (fingers, clean-ish), **Pick & Grind** (pick, pushed, 4x10
with a horn), **Downstroke Punk** (flat out, no pedals).

**Rickenbacker growl (Chris Squire, Lemmy Kilmister, and much of the 70s).** The
common thread is a bridge-pickup Rickenbacker with fresh roundwounds, played with
a pick, into an amp being asked for far more midrange than a bass amp normally
provides. Squire famously split his signal to two amps, one clean and one
distorted, which is the same idea as the clean-low/dirty-high split used in modern
bass distortion. Lemmy ran a Rickenbacker into Marshall guitar amps with bass and
treble up and mids present, effectively playing rhythm guitar an octave down.

→ **Ricken Growl** (split-band distortion, mids pushed), **Loud & Dirty**.

**Tweed-era Fender.** The 5F6-A Bassman's passive tone stack has a deep mid scoop
that gets deeper as you turn the mid control down, and its centre frequency moves
as you adjust treble. It is a genuinely interactive circuit, which is why it does
not behave like a modern active EQ. Loose, warm, breaks up early.

→ **Tweed Roundhouse**, **Blues Breakup**, **Indie Melodic**.

---

## Metal

**The split-band approach.** Almost every modern metal bass tone is two bands: a
clean low band that holds the bottom, and a heavily distorted upper band that
gives the illusion of the bass being audible through two down-tuned guitars. Some
players do this with a dedicated crossover pedal, some with a blend control, some
in the mix. If you distort the fundamental of a low B, it disappears.

→ **Split Rig Metal**, **Down-Tuned Djent**. The Low Keep control on the
distortion and fuzz models is the crossover.

**Lead bass (Cliff Burton and successors).** Big Muff-style fuzz plus a wah,
running into a cranked valve amp, with the bass carrying a melodic or solo line
rather than the bottom end.

→ **Lead Bass Fuzz**.

---

## Funk

**Envelope filters (Bootsy Collins, Larry Graham and the 70s funk school).** The
Mu-Tron III and its descendants are the sound. What matters is that the filter is
driven by playing dynamics, so the control is in the right hand: muted sixteenths
keep it shut, a popped note opens it. Bootsy also stacked octave and fuzz in front
of the filter.

→ **Envelope Funk**, **Fuzz Funk**, **Octave Funk**.

**Slap (Larry Graham, Marcus Miller, Mark King).** The tone is bright, scooped and
compressed: thumb on the low strings for the percussive thump, fingers popping the
high strings for the snap. Marcus Miller's Jazz Bass with an onboard preamp
defined the studio version. Mark King's Level 42 sound ran through Ashdown heads,
which is where the sub-harmoniser in the MAG model comes from.

→ **Slap & Pop**.

---

## Reggae and dub

**Aston "Family Man" Barrett and the Jamaican studio sound.** Flatwounds, neck
pickup, tone control fully off, played over the fingerboard, into a sealed 1x15.
The tone is deliberately the first two harmonics and nothing else. What makes it
work is note length and space, not EQ: the left hand mutes constantly.

Dub adds tape delay and spring reverb applied in the mix, usually on the off-beat.

→ **Kingston Foundation**, **Dub Delay** (tempo-synced analog delay in the amp
loop, so it locks to the metronome).

---

## Jazz and fretless

**Double-bass impression.** Dark, short and woody: low gain, 1x15, mic well back,
top end removed, and the note muted early. The decay envelope does as much work as
the EQ.

→ **Upright Impression**.

**Fretless (Jaco Pastorius).** The characteristic "mwah" is a midrange
phenomenon - it comes from the string contacting the fingerboard - so scooping the
mids destroys it. Jaco's was a 1962 Jazz Bass with the frets removed and epoxy on
the board, bridge pickup favoured, into an Acoustic 360 with its midrange boost.
He used chorus and delay occasionally, not constantly, despite the reputation.

→ **Singing Fretless**.

**Session work.** Clean DI, firm compression, nothing else.

→ **Smooth Jazz DI**.

---

## Prog and alternative

**Wal-and-effects prog (Justin Chancellor and similar).** Clean and articulate
with chorus and short delay in the loop, plus a distortion channel for heavy
sections. The low end is kept dry so chords do not lose their footing.

→ **Prog Chorus**, **Alt-Prog Grind**.

**Post-punk melodic bass (Peter Hook and the school that followed).** Played high
on the neck with a pick, bridge pickup, chorus, and a deliberately thin low end -
which is what leaves room for it to be the hook rather than the foundation.

→ **Indie Melodic**.

---

## Studio and DI

**The bass driver DI.** A preamp/DI pedal with a blend control between a clean
path and a driven path, plus a built-in speaker-emulation rolloff. It is on an
enormous number of records precisely because it needs no amp, no mic and no room.
Geddy Lee's later rigs are the well-known example: a DI preamp plus speaker
simulation, with the actual cabinets mostly there for stage monitoring.

→ **Driver DI Grit**.

**Transformer studio DI.** The high-end DI boxes used on sessions are effectively
clean - their job is impedance matching and a small amount of transformer
character on transients, not tone shaping.

→ **Flat Reference**, **Smooth Jazz DI**, **Pop Session**.

---

## What the presets are *not*

These are original models built from published circuit topologies, measured
response characteristics and a lot of listening. They are not captures of specific
units, they do not contain anyone else's impulse responses, and they are not
endorsed by or affiliated with any manufacturer. Product names appear here as
factual description of what the tone is based on.

If you want a genuine capture of your own cabinet, load your own IR - the cabinet
section takes any WAV file and runs it through the same zero-latency convolver.
