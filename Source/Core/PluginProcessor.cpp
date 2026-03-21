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
    
    samplerEngine.processBlock(buffer, midiMessages);
    
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
    
    for (auto& channel : mixerChannels)
    {
        channel->processAudio(buffer);
    }
    
    busManager.processAllBuses(buffer);
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
    samplerEngine.loadSamplesFromFolder(folder);
}

MixerChannel& DrumSampler2Processor::getMixerChannel(int index)
{
    return *mixerChannels[juce::jlimit(0, static_cast<int>(mixerChannels.size()) - 1, index)];
}

void DrumSampler2Processor::triggerNote(int midiNote, int velocity)
{
    DBG("triggerNote called - note: " << midiNote << ", velocity: " << velocity);
    
    // Queue MIDI message to be processed in audio thread
    const juce::ScopedLock sl(midiLock);
    pendingMidiMessages.addEvent(juce::MidiMessage::noteOn(1, midiNote, (juce::uint8)velocity), 0);
    
    DBG("Queued MIDI note for audio thread");
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumSampler2Processor();
}
