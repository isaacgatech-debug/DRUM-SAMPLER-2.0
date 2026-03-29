#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include "../Sampler/SamplerEngine.h"
#include "MicBusLayout.h"
#include "../Mixer/MixerChannel.h"
#include "../Mixer/BusManager.h"
#include "../Grooves/GrooveLibrary.h"
#include "../Grooves/MIDIPlayer.h"
#include "../Trigger/AudioTriggerEngine.h"
#include "../Effects/PluginManager.h"
#include "StateManager.h"
#include "PresetManager.h"

class DrumTechEditor;

class DrumTechProcessor : public juce::AudioProcessor
{
public:
    DrumTechProcessor();
    ~DrumTechProcessor() override;

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
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    const juce::AudioProcessorValueTreeState& getAPVTS() const { return apvts; }
    
    SamplerEngine& getSamplerEngine() { return samplerEngine; }
    const SamplerEngine& getSamplerEngine() const { return samplerEngine; }

    static constexpr int getNumMicBuses() noexcept { return MicBus::count; }
    BusManager& getBusManager() { return busManager; }
    MixerChannel& getMixerChannel(int index);
    MixerChannel* getMixerChannelForInput(int inputIndex);
    
    void triggerNote(int midiNote, int velocity);
    void setTransportPlaying(bool shouldPlay);
    void setTransportLooping(bool shouldLoop);
    void setTransportRecording(bool shouldRecord);
    void setTransportTempo(float bpm);
    void setTriggerThreshold(float threshold);
    bool saveCurrentPreset(const juce::String& name, const juce::String& category);
    bool loadPresetByName(const juce::String& name);

    GrooveLibrary& getGrooveLibrary() { return grooveLibrary; }
    MIDIPlayer& getMIDIPlayer() { return midiPlayer; }
    AudioTriggerEngine& getTriggerEngine() { return triggerEngine; }
    PresetManager& getPresetManager() { return presetManager; }
    PluginManager& getPluginManager() { return pluginManager; }
    
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
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void applyMixerParameters();

    struct QueuedNote
    {
        juce::uint8 note = 0;
        juce::uint8 velocity = 0;
    };

    static constexpr uint32_t kUiMidiQueueSize = 256;
    bool enqueueUiMidiNote(int midiNote, int velocity) noexcept;
    int drainUiMidiToBuffer(juce::MidiBuffer& midiMessages, int numSamples) noexcept;

    juce::AudioProcessorValueTreeState apvts;
    SamplerEngine samplerEngine;
    std::array<QueuedNote, kUiMidiQueueSize> uiMidiQueue {};
    std::atomic<uint32_t> uiMidiWrite { 0 };
    std::atomic<uint32_t> uiMidiRead { 0 };
    std::atomic<bool> recentMidiNotes[128];
    BusManager busManager;
    std::vector<std::unique_ptr<MixerChannel>> mixerChannels;
    std::array<juce::AudioBuffer<float>, MicBus::count> mixerStemBuffers;
    GrooveLibrary grooveLibrary;
    MIDIPlayer midiPlayer;
    AudioTriggerEngine triggerEngine;
    PresetManager presetManager;
    PluginManager pluginManager;
    
    juce::File lastLoadedFolder;

    bool builtinEqInserted = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumTechProcessor)
};
