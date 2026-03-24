#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "SamplerEngine.h"

class DrumLibraryProcessor : public juce::AudioProcessor
{
public:
    DrumLibraryProcessor();
    ~DrumLibraryProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Drum Library"; }
    bool acceptsMidi()  const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& dest) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void loadSamplesFromFolder (const juce::File& folder);
    int  getNumLoadedSamples() const { return engine.getNumSamples(); }
    juce::File getLastLoadedFolder() const { return lastLoadedFolder; }

private:
    SamplerEngine engine;
    juce::File    lastLoadedFolder;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumLibraryProcessor)
};
