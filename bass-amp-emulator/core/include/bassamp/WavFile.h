// WavFile.h - minimal RIFF/WAVE reader for user-supplied impulse responses.
//
// The app layer could use the host framework's audio format readers, but keeping
// IR loading in the core means the cabinet section can be tested end to end
// without an audio host, and it removes one more reason for the DSP to depend on
// anything.
#pragma once

#include "Common.h"
#include <string>
#include <vector>

namespace bassamp {

struct WavData
{
    std::vector<float> samples;   // mono mixdown
    double sampleRate = 48000.0;
    int    channels   = 1;
    bool   valid      = false;
    std::string error;
};

// Reads 8/16/24/32-bit PCM and 32/64-bit float WAV files and mixes to mono.
WavData readWavFile (const std::string& path);
WavData parseWav (const unsigned char* data, size_t size);

// Sample-rate conversion for IRs. Catmull-Rom interpolation with a pre-filter on
// downsampling; an IR is short and band-limited, so this is transparent well
// past anything a cabinet passes.
std::vector<float> resampleIr (const std::vector<float>& in, double fromRate, double toRate);

} // namespace bassamp
