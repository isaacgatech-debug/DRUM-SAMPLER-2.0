#include "PluginProcessor.h"
#include "PluginEditor.h"

DrumTechProcessor::DrumTechProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    // Initialize 12 mixer channels matching the UI inputs
    // These map to MIDI notes and drum inputs
    const int drumNotes[] = {36, 36, 38, 38, 48, 45, 50, 42, 42, 42, 49, 51};  // MIDI notes
    const char* drumNames[] = {"Kick In", "Kick Out", "Snare Top", "Snare Bottom",
                               "Tom 1", "Tom 2", "Tom 3", "OVH L", "OVH R",
                               "Hat", "RM L", "RM R"};
    
    for (int i = 0; i < 12; ++i)
    {
        mixerChannels.push_back(
            std::make_unique<MixerChannel>(drumNotes[i], drumNames[i])
        );
    }
    
    // Initialize with safe defaults so clicks work before host calls prepareToPlay
    samplerEngine.prepareToPlay(44100.0, 512);
}

DrumTechProcessor::~DrumTechProcessor()
{
}

void DrumTechProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    samplerEngine.prepareToPlay(sampleRate, samplesPerBlock);
    busManager.prepare(sampleRate, samplesPerBlock);
    
    for (auto& channel : mixerChannels)
    {
        channel->prepare(sampleRate, samplesPerBlock);
    }
}

void DrumTechProcessor::releaseResources()
{
}

void DrumTechProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    buffer.clear();
    busManager.clearAllBuses();
    
    // Merge pending MIDI messages from UI thread
    bool hadPendingMidi = false;
    {
        const juce::ScopedLock sl(midiLock);
        if (!pendingMidiMessages.isEmpty())
        {
            hadPendingMidi = true;
            DBG("*** Merging " << pendingMidiMessages.getNumEvents() << " pending MIDI messages ***");
            midiMessages.addEvents(pendingMidiMessages, 0, buffer.getNumSamples(), 0);
            pendingMidiMessages.clear();
        }
    }
    
    // Check incoming MIDI and set flags for UI
    if (!midiMessages.isEmpty() || hadPendingMidi)
    {
        DBG("*** processBlock has " << midiMessages.getNumEvents() << " MIDI events ***");
        
        // Set atomic flags for MIDI notes (thread-safe)
        for (const auto metadata : midiMessages)
        {
            auto message = metadata.getMessage();
            if (message.isNoteOn())
            {
                int note = message.getNoteNumber();
                if (note >= 0 && note < 128)
                {
                    recentMidiNotes[note].store(true);
                }
            }
        }
    }
    
    {
        const juce::ScopedLock sl(midiLock);
        DBG("*** Calling samplerEngine.processBlock ***");
        samplerEngine.processBlock(buffer, midiMessages);
        DBG("*** samplerEngine.processBlock returned ***");
    }
    
    // Check if audio was generated
    float maxLevel = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        maxLevel = juce::jmax(maxLevel, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    }
    
    if (maxLevel > 0.0f)
    {
        DBG("*** Audio generated! Max level: " << maxLevel << " ***");
    }
    else if (!midiMessages.isEmpty())
    {
        DBG("*** WARNING: MIDI received but no audio generated! ***");
    }
    
    // TEMPORARILY DISABLED - mixer channels need per-note routing, not whole-buffer processing
    // for (auto& channel : mixerChannels)
    // {
    //     DBG("*** Calling channel->processAudio ***");
    //     channel->processAudio(buffer);
    //     DBG("*** channel->processAudio returned ***");
    // }
    
    // DBG("*** Calling busManager.processAllBuses ***");
    // busManager.processAllBuses(buffer);
    // DBG("*** busManager.processAllBuses returned ***");
    
    // Keepalive: output an inaudible ~-200dB signal so Logic never suspends
    // processBlock when transport is stopped and no MIDI keyboard is active.
    // Without this, click-triggered notes queue up but never render.
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.addSample(ch, 0, 1.0e-10f);
}

juce::AudioProcessorEditor* DrumTechProcessor::createEditor()
{
    return new DrumTechEditor(*this);
}

void DrumTechProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    stream.writeString(lastLoadedFolder.getFullPathName());
}

void DrumTechProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    juce::String folderPath = stream.readString();
    
    if (folderPath.isNotEmpty())
    {
        juce::File folder(folderPath);
        if (folder.exists())
        {
            loadSamplesFromFolder(folder);
        }
    }
}

void DrumTechProcessor::loadSamplesFromFolder(const juce::File& folder)
{
    lastLoadedFolder = folder;
    DBG("*** Calling samplerEngine.loadSamplesFromFolder ***");
    samplerEngine.loadSamplesFromFolder(folder);
    DBG("*** samplerEngine.loadSamplesFromFolder returned ***");
}

MixerChannel& DrumTechProcessor::getMixerChannel(int index)
{
    return *mixerChannels[juce::jlimit(0, static_cast<int>(mixerChannels.size()) - 1, index)];
}

MixerChannel* DrumTechProcessor::getMixerChannelForInput(int inputIndex)
{
    if (inputIndex >= 0 && inputIndex < static_cast<int>(mixerChannels.size()))
        return mixerChannels[inputIndex].get();
    return nullptr;
}

void DrumTechProcessor::triggerNote(int midiNote, int velocity)
{
    DBG("*** triggerNote ENTER - note: " << midiNote << ", velocity: " << velocity << " ***");
    
    const juce::ScopedLock sl(midiLock);
    DBG("*** triggerNote: got lock, calling samplerEngine.noteOn ***");
    samplerEngine.noteOn(midiNote, velocity);
    DBG("*** triggerNote: noteOn returned ***");
    
    if (midiNote >= 0 && midiNote < 128)
        recentMidiNotes[midiNote].store(true);
        
    DBG("*** triggerNote EXIT ***");
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumTechProcessor();
}
