#include "PluginProcessor.h"
#include "PluginEditor.h"

DrumSampler2Processor::DrumSampler2Processor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    const int drumNotes[] = {36, 38, 42, 43, 45, 48, 49, 50, 51, 55, 57};
    const char* drumNames[] = {"Kick", "Snare", "Hi-Hat", "Tom 4", "Tom 3", 
                               "Tom 2", "Crash 1", "Tom 1", "Ride", "Splash", "Crash 2"};
    
    for (int i = 0; i < 11; ++i)
    {
        mixerChannels.push_back(
            std::make_unique<MixerChannel>(drumNotes[i], drumNames[i])
        );
    }
    
    // Initialize with safe defaults so clicks work before host calls prepareToPlay
    samplerEngine.prepareToPlay(44100.0, 512);
}

DrumSampler2Processor::~DrumSampler2Processor()
{
}

void DrumSampler2Processor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    samplerEngine.prepareToPlay(sampleRate, samplesPerBlock);
    busManager.prepare(sampleRate, samplesPerBlock);
    
    for (auto& channel : mixerChannels)
    {
        channel->prepare(sampleRate, samplesPerBlock);
    }
}

void DrumSampler2Processor::releaseResources()
{
}

void DrumSampler2Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

juce::AudioProcessorEditor* DrumSampler2Processor::createEditor()
{
    return new DrumSampler2Editor(*this);
}

void DrumSampler2Processor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    stream.writeString(lastLoadedFolder.getFullPathName());
}

void DrumSampler2Processor::setStateInformation(const void* data, int sizeInBytes)
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

void DrumSampler2Processor::loadSamplesFromFolder(const juce::File& folder)
{
    lastLoadedFolder = folder;
    DBG("*** Calling samplerEngine.loadSamplesFromFolder ***");
    samplerEngine.loadSamplesFromFolder(folder);
    DBG("*** samplerEngine.loadSamplesFromFolder returned ***");
}

MixerChannel& DrumSampler2Processor::getMixerChannel(int index)
{
    return *mixerChannels[juce::jlimit(0, static_cast<int>(mixerChannels.size()) - 1, index)];
}

void DrumSampler2Processor::triggerNote(int midiNote, int velocity)
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
    return new DrumSampler2Processor();
}
