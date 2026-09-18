# BassAmp Studio

A bass amp, cabinet and pedalboard emulator for Windows. Plug a 4- or 5-string
bass into a USB interface, pick a rig, and play along with a track with no
perceptible lag.

Ships as a standalone Windows app (with its own audio device selection) and as a
VST3 plugin from the same build.

---

## What is in it

**Amps** — six models, each with the controls the real thing has and none that it
does not:

| Model | Character |
|---|---|
| MAG 500 | Solid-state head. Big clean headroom, four-band EQ, Shape switch, sub-harmoniser. |
| SVT | All-valve 300 W. Two triode gain stages, selectable midrange, Ultra Lo / Ultra Hi. |
| B-15 Flip-Top | Low-powered valve combo. Very little headroom, compresses early, heavy supply sag. |
| Tweed Bassman | 5F6-A passive tone stack with its interactive mid scoop. |
| Studio DI | Transformer-coupled, essentially clean. |
| Driver DI | Bass driver preamp: blend, drive, presence, built-in speaker sim. |

The valve models include the things that actually make a valve amp feel
different: asymmetric clipping, bias shift (blocking distortion), power-supply
sag and output-transformer saturation on the low end.

**Cabinets** — 1x12, 1x15, 2x10, 2x12, 4x10, 4x12, 8x10, plus DI-only and your
own IRs. Each is a designed minimum-phase impulse response built from the cab's
low-frequency alignment, driver rolloff, cone breakup and horn, then a
microphone (four types), its position across the cone, its distance, and a little
room. Load any WAV impulse response to replace the model entirely.

**Pedals** — eighteen, on a reorderable chain, each placeable in front of the
amp, in the amp's FX loop, or after the cabinet:

compressor (opto / FET / bass-mode sidechain) · noise gate · octave · bass synth ·
fuzz · distortion · overdrive · wah · envelope filter · chorus · flanger ·
phaser · tremolo · ring mod · bit crusher · 7-band graphic EQ · delay (digital /
analog / tape) · reverb

The dirt pedals have a **Low Keep** control that routes everything below it around
the clipping stage. On bass this is the difference between a fuzz that works in a
band and one that swallows the note.

**Strobe tuner** — YIN pitch detection for the note and cents readout, plus a
genuine four-band strobe display built from quadrature demodulation. Accurate to
a couple of cents on a low B. Ten tunings including 5-string standard, BEAD,
drop tunings and piccolo, with an adjustable A reference from 415 to 466 Hz.

**Metronome** — sample-accurate, any time signature from 1/1 to 16/16, compound
meters counted correctly (6/8 in two, not six), subdivisions, swing, per-beat
accent editing, count-in and tap tempo. The delay and tremolo pedals sync to it.

**Backing track player** — load a WAV, AIFF, FLAC or MP3 and play along. The track
is mixed in after the rig, so your fuzz never touches the drums. A **Cut recorded
bass** control removes centre-panned low frequencies so the original bass part
gets out of your way.

**36 presets** across soul, rock, metal, punk, funk, reggae, jazz, prog, pop,
synth, country, blues, gospel, hip-hop and studio work, each documented with the
rig it is based on and how to play it. See
[docs/RIG_RESEARCH.md](docs/RIG_RESEARCH.md).

---

## Latency

The rig adds **zero samples**. What you feel is your interface's buffer and its
converters — around 7 ms at 48 kHz with a 128-sample ASIO buffer, or 4 ms at 64.

That is a design decision, not an accident: the cabinet convolution runs its
first 128 taps as a direct FIR so there is no partition delay, the oversampling
filters are IIR rather than linear phase, and there is no lookahead anywhere in
the dynamics section. [docs/LATENCY.md](docs/LATENCY.md) has the full budget and
the reasoning.

CPU cost is low enough that it is not the constraint. The test suite measures
throughput on every run; a typical result on a modest machine:

| Preset | Oversampling | Cost at a 128-sample buffer |
|---|---|---|
| Flat Reference | 2x | ~1.8% of one core |
| Fridge Standard | 2x | ~2.4% |
| Split Rig Metal (gate + comp + distortion) | 2x | ~2.8% |
| Split Rig Metal | 4x | ~4.0% |
| Dub Delay (comp + delay + reverb) | 2x | ~3.1% |

Which means the buffer size you can hold is set by your interface and your
drivers, not by this app.

---

## Getting it running

Full instructions, including ASIO setup, are in
[docs/BUILDING_WINDOWS.md](docs/BUILDING_WINDOWS.md). The short version:

```powershell
cd bass-amp-emulator
cmake -B build -DBASSAMP_ASIO_SDK_PATH="C:/SDKs/asiosdk"
cmake --build build --config Release --parallel
```

If you would rather not install a compiler, every push builds both formats in CI
and uploads them — grab the **BassAmpStudio-windows-x64** artifact from the
Actions tab.

**Set your input gain first.** Load the *Flat Reference* preset and adjust input
gain so hard playing peaks around -12 dBFS on the IN meter. Every other preset
assumes that.

---

## How the project is laid out

```
core/          DSP engine - plain C++17, no framework dependency
  include/bassamp/
  src/
app/Source/    JUCE layer: audio I/O, parameters, GUI
tests/         Numerical test suite for the DSP core
docs/          Build, latency and rig research notes
```

The split is deliberate. Keeping the DSP free of any framework means the parts
that are easy to get subtly wrong — the convolver's alignment, the tuner's
accuracy, the metronome's drift, the stability of every pedal at its parameter
extremes — are tested directly, on any machine, in about a second:

```bash
cmake -B build -DBASSAMP_BUILD_PLUGIN=OFF && cmake --build build && ctest --test-dir build
```

The suite currently runs 349 assertions. Among other things it verifies that the
partitioned convolver matches a direct convolution under irregular block sizes,
that the tuner reads six reference pitches from a low B upwards to within 3
cents, that the metronome does not drift by a single sample over four seconds at
any buffer size, and that all 36 presets and every pedal stay stable at their
parameter extremes.

---

## Things worth knowing

- The amp and pedal models are **original implementations** based on published
  circuit topologies and measured response characteristics. They are not
  captures of specific units and contain no third-party impulse responses.
  Product names in the documentation are factual description of what a model is
  based on, not a claim of affiliation or endorsement.
- An expression pedal sending MIDI CC 11 (configurable) drives the wah. Far
  better than dragging a slider with a bass in your hands.
- Octave, synth and sub-harmonic tracking are monophonic and analog-style: fast
  and latency-free, but they need single notes above about the fifth fret. That
  is a property of the approach, not a bug — pitch-shifting alternatives track
  chords but add latency, which defeats the point.
