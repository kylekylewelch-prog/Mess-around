#include "bassamp/Rig.h"
#include <sstream>

namespace bassamp {

void BackingTrackProcessor::prepare (double sampleRate)
{
    sideHp.setHighpass (sampleRate, 30.0f, 0.707f);
    midLowShelf.setLowShelf (sampleRate, 250.0f, 0.7f, -14.0f);
    reset();
}

void BackingTrackProcessor::reset() { sideHp.reset(); midLowShelf.reset(); }

void BackingTrackProcessor::process (float* left, float* right, int numSamples, float gain, float bassCut)
{
    const float cut = clampf (bassCut, 0.0f, 1.0f);

    for (int n = 0; n < numSamples; ++n)
    {
        float l = left[n] * gain, r = right[n] * gain;

        if (cut > 0.001f)
        {
            // Bass on a finished record is almost always dead centre, so pulling
            // the low end out of the mid channel takes the recorded bass down
            // without touching the rest of the mix much.
            const float mid  = 0.5f * (l + r);
            const float side = 0.5f * (l - r);
            const float duckedMid = lerp (mid, midLowShelf.process (mid), cut);
            const float keptSide  = sideHp.process (side);
            l = duckedMid + keptSide;
            r = duckedMid - keptSide;
        }

        left[n] = l;
        right[n] = r;
    }
}

Rig::Rig() = default;

void Rig::prepare (double sampleRate, int maxBlockSize)
{
    fs = sampleRate;
    maxBlock = std::max (1, maxBlockSize);

    ampSection.prepare (sampleRate, maxBlock, settings.oversampling);
    cabSection.prepare (sampleRate, maxBlock);
    board.prepare (sampleRate, maxBlock);
    tunerSection.prepare (sampleRate, maxBlock);
    metronomeSection.prepare (sampleRate, maxBlock);

    inputGain.reset     (sampleRate, 0.02f, dbToGain (settings.inputGainDb));
    masterGain.reset    (sampleRate, 0.02f, dbToGain (settings.masterLevelDb));
    diBlendSmooth.reset (sampleRate, 0.03f, settings.diBlend);
    tunerMute.reset     (sampleRate, 0.03f, 1.0f);

    inputMeter.reset (sampleRate);
    outputMeter.reset (sampleRate);

    work.assign ((size_t) maxBlock, 0.0f);
    direct.assign ((size_t) maxBlock, 0.0f);

    appliedCab = pendingCab;
    cabDirty = true;
    serviceMessageThread();
}

void Rig::reset()
{
    ampSection.reset();
    cabSection.reset();
    board.reset();
    tunerSection.reset();
    metronomeSection.reset();
}

void Rig::setSettings (const RigSettings& s)
{
    const bool osChanged = s.oversampling != settings.oversampling;
    settings = s;
    if (osChanged) ampSection.setOversampling (s.oversampling);
}

void Rig::requestCabinetSettings (const CabinetSettings& s)
{
    pendingCab = s;
    cabDirty = true;
}

void Rig::serviceMessageThread()
{
    if (cabDirty)
    {
        // applySettings() refuses while a previous swap is still crossfading, so
        // the flag simply stays set and we try again on the next service call.
        if (cabSection.applySettings (pendingCab))
        {
            appliedCab = pendingCab;
            cabDirty = false;
        }
    }
}

void Rig::process (const float* input, float* outLeft, float* outRight, int numSamples)
{
    if (numSamples <= 0) return;
    if ((int) work.size() < numSamples)
    {
        work.assign ((size_t) numSamples, 0.0f);
        direct.assign ((size_t) numSamples, 0.0f);
    }

    inputGain.setTarget (dbToGain (settings.inputGainDb));
    masterGain.setTarget (dbToGain (settings.masterLevelDb));
    diBlendSmooth.setTarget (clampf (settings.diBlend, 0.0f, 1.0f));
    tunerMute.setTarget ((settings.tunerActive && settings.muteWhileTuning) ? 0.0f : 1.0f);

    for (int n = 0; n < numSamples; ++n)
    {
        const float x = input[n] * inputGain.next();
        inputMeter.push (x);
        work[(size_t) n] = x;
    }

    // The tuner listens to the input regardless of the mute, so you can still
    // tune with the rig silent.
    if (settings.tunerActive) tunerSection.processAudio (work.data(), numSamples);

    for (int n = 0; n < numSamples; ++n) work[(size_t) n] *= tunerMute.next();

    board.process (work.data(), numSamples, PedalPlacement::FrontOfAmp);
    ampSection.process (work.data(), numSamples);
    board.process (work.data(), numSamples, PedalPlacement::AmpLoop);

    // Split the post-amp signal: one copy goes through the speaker, one stays
    // direct. Blending them is exactly what an engineer does with a mic and a DI.
    std::copy (work.begin(), work.begin() + numSamples, direct.begin());
    cabSection.process (work.data(), numSamples);

    for (int n = 0; n < numSamples; ++n)
    {
        const float blend = diBlendSmooth.next();
        // The DI path is hotter than the miked path because it has not been
        // through the cab's rolloffs; trim it so the blend control is usable.
        work[(size_t) n] = work[(size_t) n] * (1.0f - blend) + direct[(size_t) n] * blend * 0.5f;
    }

    board.process (work.data(), numSamples, PedalPlacement::PostCab);

    for (int n = 0; n < numSamples; ++n)
    {
        const float y = work[(size_t) n] * masterGain.next();
        outputMeter.push (y);
        outLeft[n] = y;
        outRight[n] = y;
    }

    // Monitored, not processed: the click must not go through the fuzz.
    metronomeSection.process (outLeft, numSamples);
    for (int n = 0; n < numSamples; ++n) outRight[n] = outLeft[n];
}

// ============================================================================
//  Preset serialisation
//
//  Deliberately a flat, human-readable key/value text format. Presets end up in
//  version control and in forum posts; a binary blob helps nobody.
// ============================================================================

std::string Rig::saveState() const
{
    std::ostringstream out;
    out.precision (6);
    out << std::fixed;

    out << "version=1\n";

    const AmpSettings& a = ampSection.getSettings();
    out << "amp.type="   << (int) a.type   << "\n"
        << "amp.gain="   << a.gain   << "\n"
        << "amp.master=" << a.master << "\n"
        << "amp.output=" << a.output << "\n"
        << "amp.bass="   << a.bass   << "\n"
        << "amp.mid="    << a.mid    << "\n"
        << "amp.treble=" << a.treble << "\n"
        << "amp.midfreq="<< a.midFreqIndex << "\n"
        << "amp.presence=" << a.presence << "\n"
        << "amp.blend="  << a.blend << "\n"
        << "amp.sub="    << a.subLevel << "\n"
        << "amp.bright=" << (a.bright ? 1 : 0) << "\n"
        << "amp.ultralo="<< (a.ultraLo ? 1 : 0) << "\n"
        << "amp.ultrahi="<< (a.ultraHi ? 1 : 0) << "\n"
        << "amp.shape="  << (a.shape ? 1 : 0) << "\n"
        << "amp.sag="    << a.sag << "\n";

    // Save what the user asked for, not what has been handed to the audio
    // thread yet: a cabinet swap can still be in flight when a preset is saved.
    out << "cab.type="     << (int) pendingCab.cab << "\n"
        << "cab.mic="      << (int) pendingCab.mic << "\n"
        << "cab.position=" << pendingCab.micPosition << "\n"
        << "cab.distance=" << pendingCab.micDistance << "\n"
        << "cab.room="     << pendingCab.roomAmount << "\n"
        << "cab.horn="     << (pendingCab.hornEnabled ? 1 : 0) << "\n"
        << "cab.lowcut="   << pendingCab.lowCutHz << "\n"
        << "cab.highcut="  << pendingCab.highCutHz << "\n";

    out << "rig.input="  << settings.inputGainDb << "\n"
        << "rig.master=" << settings.masterLevelDb << "\n"
        << "rig.di="     << settings.diBlend << "\n"
        << "rig.os="     << settings.oversampling << "\n";

    const auto order = board.getOrder();
    out << "chain.order=";
    for (int i = 0; i < Pedalboard::kNumSlots; ++i) out << (i ? "," : "") << order[(size_t) i];
    out << "\n";

    for (int i = 0; i < Pedalboard::kNumSlots; ++i)
    {
        const Pedal* p = const_cast<Pedalboard&> (board).pedalOfType ((PedalType) i);
        if (p == nullptr) continue;
        out << "pedal." << i << ".on=" << (p->isEnabled() ? 1 : 0) << "\n";
        out << "pedal." << i << ".place=" << (int) board.placementOf ((PedalType) i) << "\n";
        out << "pedal." << i << ".params=";
        for (int q = 0; q < p->numParams(); ++q) out << (q ? "," : "") << p->getParamValue (q);
        out << "\n";
    }

    out << "metro.bpm=" << metronomeSection.getTempo() << "\n"
        << "metro.num=" << metronomeSection.getTimeSignature().numerator << "\n"
        << "metro.den=" << metronomeSection.getTimeSignature().denominator << "\n"
        << "metro.sub=" << (int) metronomeSection.getSubdivision() << "\n"
        << "metro.swing=" << metronomeSection.getSwing() << "\n";

    return out.str();
}

namespace {

std::vector<std::string> splitCommas (const std::string& s)
{
    std::vector<std::string> out;
    std::string current;
    for (char c : s)
    {
        if (c == ',') { out.push_back (current); current.clear(); }
        else current.push_back (c);
    }
    if (! current.empty()) out.push_back (current);
    return out;
}

float toFloat (const std::string& s, float fallback = 0.0f)
{
    try { return std::stof (s); } catch (...) { return fallback; }
}

int toInt (const std::string& s, int fallback = 0)
{
    try { return std::stoi (s); } catch (...) { return fallback; }
}

} // namespace

bool Rig::loadState (const std::string& text)
{
    AmpSettings a = ampSection.getSettings();
    CabinetSettings c = pendingCab;
    RigSettings r = settings;
    int metroNum = metronomeSection.getTimeSignature().numerator;
    int metroDen = metronomeSection.getTimeSignature().denominator;
    bool sawVersion = false;

    std::istringstream in (text);
    std::string line;
    while (std::getline (in, line))
    {
        if (line.empty() || line[0] == '#') continue;
        const size_t eq = line.find ('=');
        if (eq == std::string::npos) continue;

        const std::string key = line.substr (0, eq);
        const std::string value = line.substr (eq + 1);

        if      (key == "version")      sawVersion = true;
        else if (key == "amp.type")     a.type = (AmpType) std::max (0, std::min (toInt (value), (int) AmpType::NumTypes - 1));
        else if (key == "amp.gain")     a.gain = toFloat (value);
        else if (key == "amp.master")   a.master = toFloat (value);
        else if (key == "amp.output")   a.output = toFloat (value);
        else if (key == "amp.bass")     a.bass = toFloat (value);
        else if (key == "amp.mid")      a.mid = toFloat (value);
        else if (key == "amp.treble")   a.treble = toFloat (value);
        else if (key == "amp.midfreq")  a.midFreqIndex = toInt (value);
        else if (key == "amp.presence") a.presence = toFloat (value);
        else if (key == "amp.blend")    a.blend = toFloat (value);
        else if (key == "amp.sub")      a.subLevel = toFloat (value);
        else if (key == "amp.bright")   a.bright = toInt (value) != 0;
        else if (key == "amp.ultralo")  a.ultraLo = toInt (value) != 0;
        else if (key == "amp.ultrahi")  a.ultraHi = toInt (value) != 0;
        else if (key == "amp.shape")    a.shape = toInt (value) != 0;
        else if (key == "amp.sag")      a.sag = toFloat (value);
        else if (key == "cab.type")     c.cab = (CabType) std::max (0, std::min (toInt (value), (int) CabType::NumTypes - 1));
        else if (key == "cab.mic")      c.mic = (MicType) std::max (0, std::min (toInt (value), (int) MicType::NumTypes - 1));
        else if (key == "cab.position") c.micPosition = toFloat (value);
        else if (key == "cab.distance") c.micDistance = toFloat (value);
        else if (key == "cab.room")     c.roomAmount = toFloat (value);
        else if (key == "cab.horn")     c.hornEnabled = toInt (value) != 0;
        else if (key == "cab.lowcut")   c.lowCutHz = toFloat (value, 30.0f);
        else if (key == "cab.highcut")  c.highCutHz = toFloat (value, 12000.0f);
        else if (key == "rig.input")    r.inputGainDb = toFloat (value);
        else if (key == "rig.master")   r.masterLevelDb = toFloat (value);
        else if (key == "rig.di")       r.diBlend = toFloat (value);
        else if (key == "rig.os")       r.oversampling = toInt (value, 2);
        else if (key == "metro.bpm")    metronomeSection.setTempo (toFloat (value, 120.0f));
        else if (key == "metro.num")    metroNum = toInt (value, 4);
        else if (key == "metro.den")    metroDen = toInt (value, 4);
        else if (key == "metro.sub")    metronomeSection.setSubdivision ((Subdivision) toInt (value));
        else if (key == "metro.swing")  metronomeSection.setSwing (toFloat (value));
        else if (key == "chain.order")
        {
            const auto parts = splitCommas (value);
            std::array<int, Pedalboard::kNumSlots> order { };
            for (int i = 0; i < Pedalboard::kNumSlots; ++i)
                order[(size_t) i] = (i < (int) parts.size()) ? toInt (parts[(size_t) i], i) : i;
            board.setOrder (order);
        }
        else if (key.rfind ("pedal.", 0) == 0)
        {
            const size_t firstDot = key.find ('.', 6);
            if (firstDot == std::string::npos) continue;
            const int index = toInt (key.substr (6, firstDot - 6), -1);
            if (index < 0 || index >= Pedalboard::kNumSlots) continue;

            Pedal* p = board.pedalOfType ((PedalType) index);
            if (p == nullptr) continue;

            const std::string field = key.substr (firstDot + 1);
            if (field == "on") p->setEnabled (toInt (value) != 0);
            else if (field == "place") board.setPlacement ((PedalType) index,
                                                           (PedalPlacement) std::max (0, std::min (toInt (value), 2)));
            else if (field == "params")
            {
                const auto parts = splitCommas (value);
                for (int q = 0; q < (int) parts.size() && q < p->numParams(); ++q)
                    p->setParamValue (q, toFloat (parts[(size_t) q], p->param (q).defaultValue));
            }
        }
    }

    if (! sawVersion) return false;

    metronomeSection.setTimeSignature (metroNum, metroDen);
    ampSection.setSettings (a);
    setSettings (r);
    requestCabinetSettings (c);
    serviceMessageThread();
    return true;
}

} // namespace bassamp
