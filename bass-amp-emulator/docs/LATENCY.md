# Latency: where it comes from and what this app does about it

The brief was "little to no lag while playing along with tracks". That is an
achievable target, but only if every stage is chosen with it in mind, because
latency is additive and most of it is not in the DSP.

## The budget

At 48 kHz with a 128-sample buffer, a realistic round trip looks like this:

| Stage | Typical | Notes |
|---|---|---|
| A/D conversion | 0.5 - 1.0 ms | Fixed by your interface's converters |
| Input buffer | 2.7 ms | One buffer at 128 samples |
| **This app** | **0 samples** | See below |
| Output buffer | 2.7 ms | One buffer |
| D/A conversion | 0.5 - 1.0 ms | Fixed |
| **Total** | **~7 ms** | Comparable to standing 2.5 m from your amp |

At 64 samples it is around 4 ms. For reference, the threshold where players
start to notice latency on bass is usually quoted at 10-12 ms, and you feel it
as "spongy" rather than as an echo well before you can identify it as delay.

Note what dominates: the interface, not the processing. Nothing in this app will
rescue a 512-sample buffer on a shared-mode WASAPI device.

## Design decisions that keep the app at zero

**Cabinet convolution.** The obvious way to convolve a 2048-tap impulse response
is a uniformly partitioned FFT, which costs one partition of latency - another
2.7 ms at a 128-sample partition, on top of the interface. Instead the convolver
splits the IR: the first 128 taps run as a direct time-domain FIR, available on
the sample they arrive, and the rest run through partitioned overlap-save whose
natural one-block delay is exactly the alignment those later taps need. Net
added latency: zero samples. The cost is 128 multiply-accumulates per sample,
which is nothing.

The test suite verifies this directly: an impulse in produces `ir[0]` on the very
first output sample, and the full output matches a direct convolution.

**Oversampling.** Distortion needs oversampling or it aliases audibly. The usual
choice is an FIR halfband filter - linear phase, but it costs latency, and you
pay it twice per nonlinear stage. This uses cascaded Butterworth IIR filters
instead. Phase is not linear, but the group delay in the bass range is a few tens
of microseconds, and stopband rejection at the folding frequency is better than
-70 dB, which is well below the noise floor of the gear being modelled.

**No lookahead anywhere.** The compressor and gate are feed-forward with no
lookahead buffer. A lookahead limiter would be smoother on transients and would
cost 1-5 ms; for a live rig that is the wrong trade.

**Pitch detection is off the audio thread.** YIN over a 90 ms window is far too
much work for a 64-sample callback. It runs on its own low-priority thread; the
strobe display, which is what needs to be responsive, is a quadrature
demodulation costing a handful of operations per sample.

**Cabinet changes are crossfaded, not stalled.** Switching cabinet designs a new
impulse response, which allocates and runs FFTs. That happens on the message
thread into an inactive convolver; the audio thread does a 20 ms equal-power
crossfade between the two. No allocation, no locks and no dropout on the audio
thread.

## What you control

- **Buffer size** is the single biggest lever. 128 samples is a sensible default;
  64 if your machine is stable at it.
- **Driver type**: ASIO first, WASAPI exclusive second. Nothing else.
- **Oversampling** (SETUP tab): 2x is the default and is the right setting for
  playing live. 4x is for rendering. Off roughly halves the amp section's CPU
  cost and is perfectly usable on clean and lightly driven sounds.
- **Sample rate**: 48 kHz. Running at 96 kHz halves your buffer latency in
  milliseconds but doubles the CPU load, and you will usually get further by
  halving the buffer at 48 kHz instead.

## Measuring it rather than guessing

The SETUP tab reports your actual sample rate, buffer size, the resulting
per-buffer time in milliseconds, and the plugin's reported latency (zero). If
you want the true round trip including converters, most interfaces' control
panels report it, or you can measure it with a loopback cable and any DAW's
latency calibration tool.
