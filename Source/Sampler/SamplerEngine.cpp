#include "SamplerEngine.h"
#include "../Core/ErrorLogger.h"
#include <algorithm>
#include <cmath>
#include <cstring>

#ifndef DRUMTECH_AUDIO_DEBUG
#define DRUMTECH_AUDIO_DEBUG 0
#endif

#if !DRUMTECH_AUDIO_DEBUG
#undef DBG
#define DBG(x) do {} while (false)
#endif

namespace
{
int parseVelocityToken(const juce::String& filenameLower, int& velLow, int& velHigh)
{
    auto tokens = juce::StringArray::fromTokens(filenameLower, "_-", "");
    tokens.trim();
    tokens.removeEmptyStrings();

    for (const auto& t : tokens)
    {
        if (t.startsWith("vel") && t.length() >= 6)
        {
            auto n = t.substring(3).retainCharacters("0123456789");
            if (n.isNotEmpty())
            {
                const int v = juce::jlimit(1, 127, n.getIntValue());
                velLow = juce::jmax(1, v - 6);
                velHigh = juce::jmin(127, v + 6);
                return v;
            }
        }
    }
    return -1;
}

juce::String parseArticulationToken(const juce::String& f)
{
    static const char* keys[] = {
        "tip", "side", "closed", "open", "partial",
        "bell", "rimshot", "rim", "crossstick",
        "flam", "roll", "muted", "swirl"
    };
    for (auto* k : keys)
        if (f.contains(k))
            return juce::String(k);
    return {};
}

juce::String parseStyleToken(const juce::String& f)
{
    if (f.contains("brush"))
        return "brushes";
    if (f.contains("stick"))
        return "sticks";
    return {};
}

juce::String parseDrummerToken(const juce::String& f)
{
    auto tokens = juce::StringArray::fromTokens(f, "_-", "");
    tokens.trim();
    tokens.removeEmptyStrings();
    for (const auto& t : tokens)
    {
        if (t.startsWith("drummer") && t.length() > 7)
            return t;
    }
    return {};
}
} // namespace

SamplerEngine::SamplerEngine()
{
    formatManager.registerBasicFormats();

    micTrims.fill(1.0f);

    for (int i = 0; i < 128; ++i)
    {
        pitchSettings[i] = 0.0f;
        rrCounters[i] = 0;
    }

    // Default MIDI note → mic stem when sample has no explicit micStemIndex
    noteToChannel[36] = 0;   // kick → Kick In
    noteToChannel[35] = 1;   // kick alt → Kick Out
    noteToChannel[38] = 2;   // snare → Snare Top
    noteToChannel[40] = 3;   // snare alt → Snare Bottom
    noteToChannel[42] = 14;  // hi-hat
    noteToChannel[48] = 5;   // tom 2
    noteToChannel[45] = 6;   // tom 3
    noteToChannel[50] = 4;   // tom 1
    noteToChannel[49] = 7;   // crash / OH L
    noteToChannel[57] = 8;   // crash 2 / OH R
    noteToChannel[51] = 9;   // ride → Room L bucket
    noteToChannel[43] = 5;   // tom4 → Tom 2
    noteToChannel[55] = 10;  // splash → Room R
}

int SamplerEngine::inferMicStemFromFilename(const juce::String& f) const
{
    if (f.contains("kick_in") || f.contains("kick in") || f.contains("kickin"))
        return 0;
    if (f.contains("kick_out") || f.contains("kick out") || f.contains("kickout"))
        return 1;
    if (f.contains("snare_bottom") || f.contains("snare bot") || f.contains("snarebottom"))
        return 3;
    if (f.contains("snare_top") || f.contains("snare top") || f.contains("snaretop"))
        return 2;
    if (f.contains("tom_1") || f.contains("tom1") || f.contains("racktom"))
        return 4;
    if (f.contains("tom_2") || f.contains("tom2"))
        return 5;
    if (f.contains("tom_3") || f.contains("tom3") || f.contains("floortom"))
        return 6;
    if (f.contains("tom_4") || f.contains("tom4"))
        return 5;
    if (f.contains("oh_l") || f.contains("overheadl") || f.contains("overhead_l")
        || f.contains("ovh_l") || f.contains("ohl"))
        return 7;
    if (f.contains("oh_r") || f.contains("overheadr") || f.contains("overhead_r")
        || f.contains("ovh_r") || f.contains("ohr"))
        return 8;
    if (f.contains("room_2_r") || f.contains("room2r") || f.contains("room 2 r"))
        return 11;
    if (f.contains("room_2_l") || f.contains("room2l") || f.contains("room 2 l"))
        return 12;
    if (f.contains("room_r") || f.contains("roomr") || f.contains("room r") || f.contains("rm_r"))
        return 10;
    if (f.contains("room_l") || f.contains("rooml") || f.contains("room l") || f.contains("rm_l"))
        return 9;
    if (f.contains("scotch"))
        return 13;
    if (f.contains("hihat") || f.contains("hi_hat") || f.contains("hh") || f.contains("_hat"))
        return 14;
    return -1;
}

int SamplerEngine::micTrimIndex(int midiNote, int micIndex) const
{
    const int n = juce::jlimit(0, 127, midiNote);
    const int m = juce::jlimit(0, MicBus::count - 1, micIndex);
    return n * MicBus::count + m;
}

int SamplerEngine::resolveOutputStem(const DrumSample& sample, int midiNote) const
{
    if (sample.micStemIndex >= 0 && sample.micStemIndex < MicBus::count)
        return sample.micStemIndex;
    if (auto it = noteToChannel.find(midiNote); it != noteToChannel.end())
        return juce::jlimit(0, MicBus::count - 1, it->second);
    return 0;
}

void SamplerEngine::setMicTrim(int midiNote, int micIndex, float trimMul)
{
    micTrims[static_cast<size_t>(micTrimIndex(midiNote, micIndex))] =
        juce::jmax(0.0f, juce::jmin(4.0f, trimMul));
}

float SamplerEngine::getMicTrim(int midiNote, int micIndex) const
{
    return micTrims[static_cast<size_t>(micTrimIndex(midiNote, micIndex))];
}

float SamplerEngine::getPitchForNote(int midiNote) const
{
    if (auto it = pitchSettings.find(midiNote); it != pitchSettings.end())
        return it->second;
    return 0.0f;
}

void SamplerEngine::serializeMicTrims(juce::ValueTree& state) const
{
    juce::MemoryBlock mb;
    mb.append(micTrims.data(), micTrims.size() * sizeof(float));
    state.setProperty("micTrimsBlob",
                      juce::Base64::toBase64(mb.getData(), mb.getSize()),
                      nullptr);
}

void SamplerEngine::deserializeMicTrims(const juce::ValueTree& state)
{
    micTrims.fill(1.0f);
    const auto b64 = state.getProperty("micTrimsBlob").toString();
    if (b64.isEmpty())
        return;
    juce::MemoryOutputStream mos;
    if (!juce::Base64::convertFromBase64(mos, b64))
        return;
    const size_t sz = static_cast<size_t>(mos.getDataSize());
    if (sz >= micTrims.size() * sizeof(float))
        std::memcpy(micTrims.data(), mos.getData(), micTrims.size() * sizeof(float));
}

void SamplerEngine::loadSamplesFromFolder(const juce::File& folder)
{
    samples.clear();

    if (!folder.exists())
    {
        LOG_ERROR("Sample folder does not exist: " + folder.getFullPathName());
        return;
    }

    if (!folder.isDirectory())
    {
        LOG_ERROR("Path is not a directory: " + folder.getFullPathName());
        return;
    }

    LOG_INFO("Scanning folder for samples: " + folder.getFullPathName());

    auto files = folder.findChildFiles(juce::File::findFiles, false, "*.wav;*.aif;*.aiff;*.mp3");

    LOG_INFO("Found " + juce::String(files.size()) + " audio files");

    int loadedCount = 0;

    for (const auto& file : files)
    {
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

        if (reader != nullptr)
        {
            DrumSample sample;
            sample.sampleRate = static_cast<int>(reader->sampleRate);
            sample.buffer.setSize(static_cast<int>(reader->numChannels),
                                  static_cast<int>(reader->lengthInSamples));
            reader->read(&sample.buffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

            juce::String filename = file.getFileNameWithoutExtension().toLowerCase();
            LOG_INFO("Loading sample: " + file.getFileName());

            if (filename.contains("kick") || filename.contains("bd"))
                sample.midiNote = 36;
            else if (filename.contains("snare") || filename.contains("sd"))
                sample.midiNote = 38;
            else if (filename.contains("hihat") || filename.contains("hh") || filename.contains("hat"))
                sample.midiNote = 42;
            else if (filename.contains("tom1") || filename.contains("racktom"))
                sample.midiNote = 50;
            else if (filename.contains("tom2"))
                sample.midiNote = 48;
            else if (filename.contains("tom3") || filename.contains("floortom"))
                sample.midiNote = 45;
            else if (filename.contains("tom4"))
                sample.midiNote = 43;
            else if (filename.contains("crash1") || filename.contains("crash"))
                sample.midiNote = 49;
            else if (filename.contains("ride"))
                sample.midiNote = 51;
            else if (filename.contains("splash"))
                sample.midiNote = 55;
            else if (filename.contains("crash2"))
                sample.midiNote = 57;
            else
            {
                LOG_WARNING("Could not auto-map sample to MIDI note: " + file.getFileName());
                sample.midiNote = 60;
            }

            sample.micStemIndex = inferMicStemFromFilename(filename);
            sample.articulationKey = parseArticulationToken(filename);
            sample.playingStyleKey = parseStyleToken(filename);
            sample.drummerProfileKey = parseDrummerToken(filename);

            int vLow = 0, vHigh = 127;
            if (parseVelocityToken(filename, vLow, vHigh) >= 0)
            {
                sample.velLow = vLow;
                sample.velHigh = vHigh;
            }

            auto rrPos = filename.indexOfIgnoreCase("rr");
            if (rrPos >= 0)
            {
                auto rrNum = filename.substring(rrPos + 2).retainCharacters("0123456789");
                if (rrNum.isNotEmpty())
                    sample.rrGroup = juce::jmax(0, rrNum.getIntValue());
            }

            LOG_INFO("Mapped " + file.getFileName() + " to MIDI note " + juce::String(sample.midiNote)
                     + " micStem " + juce::String(sample.micStemIndex)
                     + " vel " + juce::String(sample.velLow) + "-" + juce::String(sample.velHigh));

            samples.push_back(std::move(sample));
            loadedCount++;
        }
        else
        {
            LOG_ERROR("Failed to read audio file: " + file.getFileName());
        }
    }

    LOG_INFO("Successfully loaded " + juce::String(loadedCount) + " of " + juce::String(files.size())
             + " samples");
}

void SamplerEngine::prepareToPlay(double sampleRate, int blockSize)
{
    DBG("*** SamplerEngine::prepareToPlay ENTER - sampleRate=" << sampleRate << " ***");
    currentSampleRate = sampleRate;
    for (int i = 0; i < MAX_VOICES; ++i)
    {
        voices[i].setSampleRate(sampleRate);
        DBG("*** Voice " << i << " sampleRate set to " << sampleRate << " ***");
    }

    for (auto& channelBuffer : channelBuffers)
    {
        channelBuffer.setSize(2, blockSize, false, false, true);
        channelBuffer.clear();
    }
    DBG("*** SamplerEngine::prepareToPlay EXIT ***");
}

void SamplerEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    buffer.clear();
    for (auto& channelBuffer : channelBuffers)
        channelBuffer.clear();

    bool hadMidiNote = false;

    for (const auto metadata : midi)
    {
        auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            hadMidiNote = true;
            int note = message.getNoteNumber();
            DBG("*** MIDI NOTE ON: " << note << " velocity: " << message.getVelocity() << " ***");
            DBG("*** Number of samples loaded: " << samples.size() << " ***");

            noteOn(note, message.getVelocity());
        }
        else if (message.isNoteOff())
        {
            noteOff(message.getNoteNumber());
        }
    }

    int activeVoices = 0;
    for (auto& voice : voices)
    {
        if (voice.isActive())
        {
            activeVoices++;
            const int outputChannel = juce::jlimit(0, NUM_DRUM_CHANNELS - 1, voice.getOutputChannel());
            voice.process(channelBuffers[static_cast<size_t>(outputChannel)], 0, buffer.getNumSamples());
        }
    }

    for (const auto& channelBuffer : channelBuffers)
    {
        const int channels = juce::jmin(buffer.getNumChannels(), channelBuffer.getNumChannels());
        const int nSamps = juce::jmin(buffer.getNumSamples(), channelBuffer.getNumSamples());
        for (int ch = 0; ch < channels; ++ch)
            buffer.addFrom(ch, 0, channelBuffer, ch, 0, nSamps);
    }

    if (hadMidiNote)
    {
        DBG("*** Active voices after MIDI: " << activeVoices << " ***");

        float maxLevel = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            maxLevel = juce::jmax(maxLevel, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
        }
        DBG("*** Buffer max level: " << maxLevel << " ***");
    }
}

void SamplerEngine::noteOn(int midiNote, int velocity)
{
    DBG("*** SamplerEngine::noteOn ENTER - note: " << midiNote << ", velocity: " << velocity << " ***");
    DBG("*** currentSampleRate: " << currentSampleRate << " ***");
    DBG("*** Total samples loaded: " << samples.size() << " ***");

    auto collectCandidates = [this, midiNote, velocity](bool enforceArticulation)
    {
        std::vector<const DrumSample*> exact;
        std::vector<const DrumSample*> nearest;
        int bestVelDistance = 9999;
        const auto artHint = getArticulationHintForNote(midiNote);

        for (const auto& sample : samples)
        {
            DBG("*** Checking sample: note=" << sample.midiNote << " vel=" << sample.velLow << "-"
                                             << sample.velHigh << " ***");
            if (sample.midiNote != midiNote)
                continue;

            if (currentPlayingStyle != "auto"
                && sample.playingStyleKey.isNotEmpty()
                && sample.playingStyleKey != currentPlayingStyle)
                continue;

            if (currentDrummerProfile != "default"
                && sample.drummerProfileKey.isNotEmpty()
                && sample.drummerProfileKey != currentDrummerProfile)
                continue;

            if (enforceArticulation
                && artHint.isNotEmpty()
                && sample.articulationKey.isNotEmpty()
                && sample.articulationKey != artHint)
                continue;

            if (velocity >= sample.velLow && velocity <= sample.velHigh)
            {
                exact.push_back(&sample);
                continue;
            }

            const int center = (sample.velLow + sample.velHigh) / 2;
            const int dist = std::abs(velocity - center);
            if (dist < bestVelDistance)
            {
                bestVelDistance = dist;
                nearest.clear();
                nearest.push_back(&sample);
            }
            else if (dist == bestVelDistance)
            {
                nearest.push_back(&sample);
            }
        }

        if (!exact.empty())
            return exact;
        return nearest;
    };

    auto candidates = collectCandidates(true);
    if (candidates.empty())
        candidates = collectCandidates(false);

    DBG("*** Found " << candidates.size() << " candidate samples ***");

    if (candidates.empty())
    {
        DBG("*** ERROR: No sample found for MIDI note " << midiNote << " ***");
        return;
    }

    int rrIndex = rrCounters[midiNote] % static_cast<int>(candidates.size());
    const DrumSample* selectedSample = candidates[static_cast<size_t>(rrIndex)];
    rrCounters[midiNote]++;

    DBG("*** Selected sample with " << selectedSample->buffer.getNumSamples() << " samples at "
                                    << selectedSample->sampleRate << "Hz ***");

    DrumVoice* freeVoice = nullptr;
    int voiceIndex = -1;
    for (int i = 0; i < MAX_VOICES; ++i)
    {
        if (!voices[i].isActive())
        {
            freeVoice = &voices[i];
            voiceIndex = i;
            break;
        }
    }

    if (freeVoice == nullptr)
    {
        DBG("*** No free voices, stealing voice 0 ***");
        freeVoice = &voices[0];
        voiceIndex = 0;
    }
    else
    {
        DBG("*** Using voice " << voiceIndex << " ***");
    }

    float gain = velocity / 127.0f;

    const int stem = resolveOutputStem(*selectedSample, midiNote);
    const float trim = getMicTrim(midiNote, stem);

    if (auto pitchIt = pitchSettings.find(midiNote); pitchIt != pitchSettings.end())
        freeVoice->setPitch(pitchIt->second);
    if (auto velIt = velocityCurves.find(midiNote); velIt != velocityCurves.end())
        freeVoice->setVelocityCurve(velIt->second);

    DBG("*** Triggering voice " << voiceIndex << " with gain " << gain << " stem " << stem << " ***");
    freeVoice->trigger(selectedSample, gain, stem, trim);
    DBG("*** Voice " << voiceIndex << " isActive after trigger: " << freeVoice->isActive() << " ***");
}

void SamplerEngine::noteOff(int midiNote)
{
    for (auto& voice : voices)
    {
        if (voice.isActive())
            voice.stop();
    }
    juce::ignoreUnused(midiNote);
}

void SamplerEngine::setPitchForNote(int midiNote, float semitones)
{
    pitchSettings[midiNote] = semitones;
}

void SamplerEngine::setVelocityCurveForNote(int midiNote, const VelocityCurve& curve)
{
    velocityCurves[midiNote] = curve;
}

VelocityCurve SamplerEngine::getVelocityCurveForNote(int midiNote) const
{
    if (auto it = velocityCurves.find(midiNote); it != velocityCurves.end())
        return it->second;
    return {};
}

void SamplerEngine::setChannelForNote(int midiNote, int channel)
{
    noteToChannel[midiNote] = juce::jlimit(0, MicBus::count - 1, channel);
}

void SamplerEngine::setArticulationHintForNote(int midiNote, const juce::String& articulationKey)
{
    auto key = articulationKey.trim().toLowerCase();
    if (key.isEmpty() || key == "auto")
        articulationHints.erase(midiNote);
    else
        articulationHints[midiNote] = key;
}

juce::String SamplerEngine::getArticulationHintForNote(int midiNote) const
{
    if (auto it = articulationHints.find(midiNote); it != articulationHints.end())
        return it->second;
    return {};
}

void SamplerEngine::setDrummerProfile(const juce::String& profileKey)
{
    currentDrummerProfile = profileKey.toLowerCase().isEmpty()
        ? juce::String("default")
        : profileKey.toLowerCase();
}

void SamplerEngine::setPlayingStyle(const juce::String& styleKey)
{
    auto k = styleKey.toLowerCase();
    if (k != "sticks" && k != "brushes")
        k = "auto";
    currentPlayingStyle = k;
}
