#include "PluginProcessor.h"
#include "PluginEditor.h"

DrumLibraryProcessor::DrumLibraryProcessor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

DrumLibraryProcessor::~DrumLibraryProcessor() {}

void DrumLibraryProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepareToPlay (sampleRate, samplesPerBlock);
}

void DrumLibraryProcessor::releaseResources() {}

void DrumLibraryProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    engine.processBlock (buffer, midi);
}

juce::AudioProcessorEditor* DrumLibraryProcessor::createEditor()
{
    return new DrumLibraryEditor (*this);
}

void DrumLibraryProcessor::loadSamplesFromFolder (const juce::File& folder)
{
    lastLoadedFolder = folder;
    engine.loadSamplesFromFolder (folder);
}

void DrumLibraryProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::MemoryOutputStream stream (dest, true);
    stream.writeString (lastLoadedFolder.getFullPathName());
}

void DrumLibraryProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream (data, (size_t) sizeInBytes, false);
    juce::File folder (stream.readString());
    if (folder.isDirectory())
        loadSamplesFromFolder (folder);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumLibraryProcessor();
}
