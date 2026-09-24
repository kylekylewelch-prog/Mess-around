// Convolver.h - zero-latency convolution for the cabinet section.
//
// A plain uniform-partitioned FFT convolver adds one partition of latency. That
// is the wrong trade for a live rig: at 48 kHz a 128-sample partition is another
// 2.7 ms on top of the interface round trip, and you feel it against a backing
// track. So this is the Gardner hybrid:
//
//   * the first `headLength` taps run as a direct time-domain FIR - exact, and
//     available on the very sample they arrive;
//   * the remaining taps run through uniform-partitioned overlap-save, whose
//     natural one-block delay is exactly the alignment those taps need.
//
// Net additional latency: zero samples. The cost is headLength MACs per sample,
// which for 128 taps is nothing on any CPU that can run a DAW.
#pragma once

#include "Common.h"
#include "Fft.h"
#include <vector>

namespace bassamp {

class Convolver
{
public:
    void prepare (double sampleRate, int maxBlockSize, int partitionSize = 128);

    // Swaps in a new impulse response. Not real-time safe (it allocates and runs
    // FFTs); the audio thread must not call it directly - the rig stages IR
    // changes from the message thread behind a crossfade.
    void setImpulseResponse (const float* ir, int numSamples);

    void reset();

    // In-place block processing.
    void process (float* data, int numSamples);

    int  getIrLength() const { return irLength; }
    bool isActive()    const { return irLength > 0; }

private:
    void pushFftBlock();

    Fft fft;
    int partition = 128, fftSize = 256, numPartitions = 0, headLength = 128, irLength = 0;

    std::vector<float> head;          // direct FIR taps
    std::vector<float> headDelay;     // circular buffer for the direct section
    int headWritePos = 0;

    std::vector<float> filterRe, filterIm;   // partitioned spectra, numPartitions * fftSize
    std::vector<float> fdlRe, fdlIm;         // frequency-domain delay line of input blocks
    int fdlIndex = 0;

    std::vector<float> inputBlock;    // current partition being filled
    int inputFill = 0;
    std::vector<float> overlap;       // overlap-save carry (partition samples)
    std::vector<float> tailOut;       // FFT-section output waiting to be consumed
    int tailRead = 0;

    std::vector<float> workRe, workIm, accRe, accIm;
};

} // namespace bassamp
