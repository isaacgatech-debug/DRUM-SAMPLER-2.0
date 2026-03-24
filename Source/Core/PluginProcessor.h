#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "../Sampler/SamplerEngine.h"
#include "../Mixer/MixerChannel.h"
#include "../Mixer/BusManager.h"

class DrumSampler2Editor;

class DrumSampler2Processor : public juce::AudioProcessor
{
public:
    DrumSampler2Processor();
    ~DrumSampler2Processor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Drum Tech"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    bool silenceInProducesSilenceOut() const override { return false; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void loadSamplesFromFolder(const juce::File& folder);
    int getNumLoadedSamples() const { return samplerEngine.getNumSamples(); }
    
    SamplerEngine& getSamplerEngine() { return samplerEngine; }
    BusManager& getBusManager() { return busManager; }
    MixerChannel& getMixerChannel(int index);
    MixerChannel* getMixerChannelForInput(int inputIndex);
    
    void triggerNote(int midiNote, int velocity);
    
    // Thread-safe way to check if a MIDI note was recently triggered
    bool checkAndClearMidiNote(int midiNote)
    {
        if (midiNote >= 0 && midiNote < 128)
        {
            return recentMidiNotes[midiNote].exchange(false);
        }
        return false;
    }

private:
    SamplerEngine samplerEngine;
    juce::MidiBuffer pendingMidiMessages;
    juce::CriticalSection midiLock;
    std::atomic<bool> recentMidiNotes[128];
    BusManager busManager;
    std::vector<std::unique_ptr<MixerChannel>> mixerChannels;
    
    juce::File lastLoadedFolder;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumSampler2Processor)
};
