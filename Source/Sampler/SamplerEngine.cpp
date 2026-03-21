#include "SamplerEngine.h"
#include "../Core/ErrorLogger.h"
#include <algorithm>

SamplerEngine::SamplerEngine()
{
    formatManager.registerBasicFormats();
    
    for (int i = 0; i < 128; ++i)
    {
        pitchSettings[i] = 0.0f;
        rrCounters[i] = 0;
    }
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
                sample.midiNote = 60; // Default to middle C
            }
            
            LOG_INFO("Mapped " + file.getFileName() + " to MIDI note " + juce::String(sample.midiNote));
            
            samples.push_back(std::move(sample));
            loadedCount++;
        }
        else
        {
            LOG_ERROR("Failed to read audio file: " + file.getFileName());
        }
    }
    
    LOG_INFO("Successfully loaded " + juce::String(loadedCount) + " of " + juce::String(files.size()) + " samples");
}

void SamplerEngine::prepareToPlay(double sampleRate, int blockSize)
{
    currentSampleRate = sampleRate;
}

void SamplerEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    buffer.clear();
    
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
    
    // Process voices
    int activeVoices = 0;
    for (auto& voice : voices)
    {
        if (voice.isActive())
        {
            activeVoices++;
            voice.process(buffer, 0, buffer.getNumSamples());
        }
    }
    
    if (hadMidiNote)
    {
        DBG("*** Active voices after MIDI: " << activeVoices << " ***");
        
        // Check if any audio was generated
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
    DBG("MIDI Note On: " << midiNote << " velocity: " << velocity);
    
    std::vector<const DrumSample*> candidates;
    
    for (const auto& sample : samples)
    {
        if (sample.midiNote == midiNote &&
            velocity >= sample.velLow &&
            velocity <= sample.velHigh)
        {
            candidates.push_back(&sample);
        }
    }
    
    if (candidates.empty())
    {
        DBG("No sample found for MIDI note " << midiNote);
        return;
    }
    
    DBG("Found " << candidates.size() << " candidate samples for note " << midiNote);
    
    int rrIndex = rrCounters[midiNote] % static_cast<int>(candidates.size());
    const DrumSample* selectedSample = candidates[rrIndex];
    rrCounters[midiNote]++;
    
    DrumVoice* freeVoice = nullptr;
    for (auto& voice : voices)
    {
        if (!voice.isActive())
        {
            freeVoice = &voice;
            break;
        }
    }
    
    if (freeVoice == nullptr)
    {
        DBG("No free voices, stealing voice 0");
        freeVoice = &voices[0];
    }
    
    float gain = velocity / 127.0f;
    
    int channel = 0;
    if (noteToChannel.find(midiNote) != noteToChannel.end())
        channel = noteToChannel[midiNote];
    
    DBG("Triggering voice with gain " << gain << " on channel " << channel);
    freeVoice->trigger(selectedSample, gain, channel);
    
    if (pitchSettings.find(midiNote) != pitchSettings.end())
        freeVoice->setPitch(pitchSettings[midiNote]);
    
    if (velocityCurves.find(midiNote) != velocityCurves.end())
        freeVoice->setVelocityCurve(velocityCurves[midiNote]);
}

void SamplerEngine::noteOff(int midiNote)
{
    for (auto& voice : voices)
    {
        if (voice.isActive())
            voice.stop();
    }
}

void SamplerEngine::setPitchForNote(int midiNote, float semitones)
{
    pitchSettings[midiNote] = semitones;
}

void SamplerEngine::setVelocityCurveForNote(int midiNote, const VelocityCurve& curve)
{
    velocityCurves[midiNote] = curve;
}

void SamplerEngine::setChannelForNote(int midiNote, int channel)
{
    noteToChannel[midiNote] = channel;
}
