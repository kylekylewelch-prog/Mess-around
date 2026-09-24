#include "bassamp/WavFile.h"
#include "bassamp/Filters.h"
#include <cstring>
#include <fstream>

namespace bassamp {

static uint32_t readU32 (const unsigned char* p) { return (uint32_t) p[0] | ((uint32_t) p[1] << 8) | ((uint32_t) p[2] << 16) | ((uint32_t) p[3] << 24); }
static uint16_t readU16 (const unsigned char* p) { return (uint16_t) ((uint16_t) p[0] | ((uint16_t) p[1] << 8)); }

WavData parseWav (const unsigned char* data, size_t size)
{
    WavData out;

    if (size < 44 || std::memcmp (data, "RIFF", 4) != 0 || std::memcmp (data + 8, "WAVE", 4) != 0)
    {
        out.error = "Not a RIFF/WAVE file";
        return out;
    }

    size_t pos = 12;
    uint16_t format = 0, channels = 0, bits = 0;
    uint32_t sampleRate = 0;
    const unsigned char* dataChunk = nullptr;
    uint32_t dataSize = 0;

    while (pos + 8 <= size)
    {
        const char* id = (const char*) (data + pos);
        const uint32_t chunkSize = readU32 (data + pos + 4);
        const size_t body = pos + 8;

        if (std::memcmp (id, "fmt ", 4) == 0 && body + 16 <= size)
        {
            format     = readU16 (data + body);
            channels   = readU16 (data + body + 2);
            sampleRate = readU32 (data + body + 4);
            bits       = readU16 (data + body + 14);
            // WAVE_FORMAT_EXTENSIBLE stores the real format tag in the subformat GUID.
            if (format == 0xFFFE && body + 26 <= size) format = readU16 (data + body + 24);
        }
        else if (std::memcmp (id, "data", 4) == 0)
        {
            dataChunk = data + body;
            dataSize  = (uint32_t) std::min ((size_t) chunkSize, size - body);
        }

        pos = body + chunkSize + (chunkSize & 1);   // chunks are word aligned
    }

    if (dataChunk == nullptr || channels == 0)
    {
        out.error = "Missing fmt/data chunk";
        return out;
    }
    if (format != 1 && format != 3)
    {
        out.error = "Only PCM and IEEE float WAV files are supported (compressed files are not)";
        return out;
    }

    const int bytesPerSample = bits / 8;
    if (bytesPerSample <= 0)
    {
        out.error = "Unsupported bit depth";
        return out;
    }

    const size_t frames = dataSize / (size_t) (bytesPerSample * channels);
    out.samples.resize (frames, 0.0f);
    out.channels = channels;
    out.sampleRate = sampleRate > 0 ? (double) sampleRate : 48000.0;

    for (size_t f = 0; f < frames; ++f)
    {
        float sum = 0.0f;
        for (int c = 0; c < channels; ++c)
        {
            const unsigned char* s = dataChunk + (f * (size_t) channels + (size_t) c) * (size_t) bytesPerSample;
            float v = 0.0f;
            if (format == 3)
            {
                if (bits == 32) { float tmp; std::memcpy (&tmp, s, 4); v = tmp; }
                else if (bits == 64) { double tmp; std::memcpy (&tmp, s, 8); v = (float) tmp; }
            }
            else
            {
                switch (bits)
                {
                    case 8:  v = ((float) s[0] - 128.0f) / 128.0f; break;
                    case 16: v = (float) (int16_t) readU16 (s) / 32768.0f; break;
                    case 24: {
                        int32_t i = (int32_t) ((uint32_t) s[0] | ((uint32_t) s[1] << 8) | ((uint32_t) s[2] << 16));
                        if (i & 0x800000) i |= ~0xFFFFFF;   // sign extend
                        v = (float) i / 8388608.0f;
                        break;
                    }
                    case 32: v = (float) (int32_t) readU32 (s) / 2147483648.0f; break;
                    default: break;
                }
            }
            sum += v;
        }
        out.samples[f] = sum / (float) channels;
    }

    out.valid = true;
    return out;
}

WavData readWavFile (const std::string& path)
{
    WavData out;
    std::ifstream in (path, std::ios::binary);
    if (! in)
    {
        out.error = "Could not open " + path;
        return out;
    }
    std::vector<unsigned char> bytes ((std::istreambuf_iterator<char> (in)), std::istreambuf_iterator<char>());
    if (bytes.size() < 44)
    {
        out.error = "File is too short to be a WAV";
        return out;
    }
    return parseWav (bytes.data(), bytes.size());
}

std::vector<float> resampleIr (const std::vector<float>& in, double fromRate, double toRate)
{
    if (in.empty() || fromRate <= 0.0 || toRate <= 0.0 || std::fabs (fromRate - toRate) < 1.0)
        return in;

    std::vector<float> src = in;

    // Band-limit before decimating, otherwise the top octave folds back into the
    // cabinet's presence region where it is very audible.
    if (toRate < fromRate)
    {
        Biquad a, b;
        const float cutoff = (float) (toRate * 0.45);
        a.setLowpass (fromRate, cutoff, 0.54f);
        b.setLowpass (fromRate, cutoff, 1.31f);
        for (auto& v : src) v = b.process (a.process (v));
    }

    const double ratio = fromRate / toRate;
    const size_t outLen = (size_t) ((double) src.size() / ratio);
    std::vector<float> out (outLen, 0.0f);

    auto at = [&src] (long i) -> float
    {
        if (i < 0 || i >= (long) src.size()) return 0.0f;
        return src[(size_t) i];
    };

    for (size_t n = 0; n < outLen; ++n)
    {
        const double x = (double) n * ratio;
        const long   i = (long) x;
        const float  t = (float) (x - (double) i);
        const float p0 = at (i - 1), p1 = at (i), p2 = at (i + 1), p3 = at (i + 2);
        // Catmull-Rom
        out[n] = 0.5f * ((2.0f * p1)
                        + (-p0 + p2) * t
                        + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t
                        + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t);
    }
    return out;
}

} // namespace bassamp
