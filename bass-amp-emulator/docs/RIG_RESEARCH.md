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

## Signature rigs

These are built from a specific player's documented setup rather than a genre
convention. Where a player changed rigs substantially between eras, there is a
preset per era, because the tones genuinely are not interchangeable.

### Geddy Lee

Three distinct eras, two of which are modelled.

**1976-1984: Rickenbacker 4001 into Ampeg SVT.** The 4001 has stereo outputs, and
he ran the bridge pickup into a dirtier amp than the neck. What people mean by
"the Geddy tone" on *Moving Pictures* is almost entirely upper midrange: roughly
800 Hz to 2 kHz, with the SVT pushed far enough to thicken on attack. The low end
is present but not the point. *Tom Sawyer* and *YYZ* are the reference - on both,
the bass occupies the band a guitar normally would, which is how it stays audible
in a three-piece. *Limelight* adds audible chorus.

→ **Clank & Growl**. Ultra Hi engaged, midrange selector at 1.6 kHz, bass control
deliberately below halfway.

**1986-1992: Wal Mk II into a rack preamp.** Active electronics, much tighter and
more scooped, and the attack rather than the distortion carrying the aggression.
*Power Windows* and *Hold Your Fire*. This is a hi-fi sound and a fair bit of
80s chorus is part of it.

→ **Eighties Hi-Fi**.

**1993 onwards: Fender Jazz, and later three Orange AD200B heads into 8x10s.**
Covered adequately by **Driver DI Grit** and **Fridge Standard**; his later live
sound was largely a DI preamp with speaker simulation, with the cabs on stage for
monitoring.

### Billy Sheehan

The defining feature is not a pedal, it is the instrument wiring. His modified
Precision and the Yamaha Attitude signatures that followed have **two separate
outputs**: a neck humbucker feeding a clean bass rig, and a bridge pickup feeding
a distorted guitar-style amp. The low end never goes through the distortion at
all.

The Low Keep control on this plugin's dirt pedals does the same job as his
crossover, which is why the split is modelled with a single distortion pedal
rather than two chains. *Shy Boy* (Talas, then *Eat 'Em and Smile*, 1986) and
*Addicted to That Rush* (Mr. Big, 1989) are the reference for the heavy version;
the three-finger right hand and the wide bends need the upper mids intact or fast
passages stop being legible.

→ **Attitude Split** (rhythm), **Three-Finger Lead** (lead and tapping).

### John Paul Jones

**Led Zeppelin I-IV: Fender Jazz into an Acoustic 360/361.** The 360 is a
solid-state preamp driving an 18-inch folded-horn cabinet, with a built-in fuzz
and a variamp tone section. The folded horn is why the low end on *Led Zeppelin
II* is so deep, and the cabinet's midrange honk around 350 Hz is why the bass is
so easy to follow despite that depth. *The Lemon Song* is the clearest example -
the bass is effectively a second lead instrument - and *Ramble On* is the same
voice used melodically. The plugin has no 1x18, so the 1x15 model with the
midrange selector at 350 Hz is the closest approximation.

→ **Folded Horn**.

**Presence and after: Alembic Series I.** Through-neck, active, with low-pass
filter electronics - a much more modern and hi-fi instrument, extended at both
ends. *Achilles Last Stand* is the reason to model it separately: sustained
sixteenth-note playing at that tempo needs clarity the 360 rig does not have.

→ **Alembic Hi-Fi**.

### Geezer Butler

**Black Sabbath, 1970.** Precision into Laney heads and guitar 4x12 cabinets -
the same amplification Iommi used, which is most of why early Sabbath bass sounds
more like a guitar than a bass rig. Accounts of the exact fuzz differ; a Fuzz Face
derivative is the most commonly cited, and the audible result is a gated,
midrange-heavy saturation rather than a modern scooped fuzz.

*N.I.B.* opens with **"Bassically"**, an unaccompanied bass intro built on fuzz
into a wah. It is worth separating from the body of the song: the intro is a solo
tone and the riff tone is not. Both are in E; the band did not drop to C# until
*Master of Reality*.

→ **Bassically** (fuzz plus wah, intro), **Birmingham Riff** (the song body).

The Bassman model is used for both because Laney's early circuits descend from the
same 5F6-A lineage as Marshall's.

### James Jamerson

Already covered under Motown, but two further points justify separate presets.

The recordings are **console tracks first and amp tracks second**. Hitsville's
chain put the bass through a custom DI and valve limiting, and on most of the
famous sides the DI dominates. The limiting is heavier than sounds correct in
isolation - *What's Going On*, *For Once in My Life* and *Bernadette* all have a
bass part with almost no dynamic range and very little content above 1 kHz.

→ **Snakepit Direct**.

The second point is practical: that tone does not survive a stage. His busiest
lines need 400 to 800 Hz opened back up to stay legible in a live band, without
adding the top end that would break the illusion.

→ **Hook Melodic**.

### Tim Commerford

**Music Man StingRay into an Ampeg SVT, with a separate driven amp blended in.**
The StingRay's active two-band EQ and bridge-position humbucker give a pronounced
low-mid punch and an aggressive top; the dirt is parallel, not in series, which is
why the fundamental survives on *Bombtrack* and the *Killing in the Name*
breakdown while the grind still reads as distortion.

*Take the Power Back* is the other half of the same instrument: clean, scooped,
and carried by sixteenth-note ghost notes. That needs fast compression and the
Ultra Lo scoop rather than drive.

→ **Stingray Grind**, **Rap-Rock Snap**.

### Ryan Martinie

**Warwick Thumb into Ampeg and Mesa amplification.** The Thumb's wenge neck and
bubinga body produce a hard midrange bark between roughly 800 Hz and 1.6 kHz, and
that bark is the entire reason the bass is audible over down-tuned guitars on
*L.D. 50*. *Dig* is the showcase - slap, pop and tapped passages all in one part -
and it needs fast compression with the upper mids left alone.

The clean side matters as much: *World So Cold* and *Death Blooms* have the bass
carrying melody over the guitars, chorused and sustained, with the low end kept
dry so the bottom stays solid.

→ **Thumb Bark**, **Melodic Over Drop**.

---

## Latin rock

**Santana, Abraxas era (David Brown).** Precision into a flip-top or Bassman-style
valve amp. The requirement is specific: a tumbao pattern is felt through the
*anticipated* notes - the and of two, and beat four - so those need length and
clear fundamental while everything else stays short. Congas and timbales occupy
everything above about 2 kHz, so the bass gives that range up entirely. *Oye Como
Va* and *Black Magic Woman* are the reference.

→ **Tumbao**.

**Chicano rock and Latin funk.** Same instrument, more midrange and more attack,
because here the bass line is usually the hook rather than the foundation. *Low
Rider* is the obvious case.

→ **Barrio Groove**.

---

## Classic rock

**John Entwistle.** Effectively invented the bright roundwound bass sound, and
bi-amped so the highs went to guitar amplification. Almost no compression, fresh
Rotosound roundwounds, and a deliberate willingness to let the strings hit the
frets - the clank is the sound, not an artefact. *My Generation* and *The Real Me*.

→ **Typewriter**.

**The seventies session default.** Precision with worn strings into a valve head
backed off below breakup, mic well off the grille. Warm without being dull: the
treble is down and the mids are not.

→ **Seventies Session**.

**Harmonised twin-guitar bands (Thin Lizzy and the lineage after it).** Two
guitars already own the midrange, so the bass has to claim a narrow band - around
400 Hz - and hold it, with just enough overdrive to be heard as a distinct part.

→ **Twin Guitar Grit**.

---

## Emo and alt

**Pick-played emo and pop-punk.** Precision with a pick into a valve head with
mild overdrive. The pick attack is doing real work: it is what keeps the bass
audible when two overdriven guitars are playing the same chord voicings. Jimmy
Eat World, The Get Up Kids, and Nate Mendel's playing in Sunny Day Real Estate and
after.

→ **Basement Pick**.

**The clean counterpart.** Arpeggiated verses where the bass plays a countermelody
- chorused, bright, fingerstyle, notes ringing into each other. American Football
and Sunny Day Real Estate. The chorus keeps the low end dry so the bottom stays
solid under the shimmer.

→ **Midwest Clean**.

**The louder end.** More saturation, with the low band split out of the dirt so
the drops still land. Thursday, Brand New, Taking Back Sunday.

→ **Post-Hardcore Drive**.

---

## What the presets are *not*

These are original models built from published circuit topologies, measured
response characteristics and a lot of listening. They are not captures of specific
units, they do not contain anyone else's impulse responses, and they are not
endorsed by or affiliated with any manufacturer. Product names appear here as
factual description of what the tone is based on.

If you want a genuine capture of your own cabinet, load your own IR - the cabinet
section takes any WAV file and runs it through the same zero-latency convolver.
