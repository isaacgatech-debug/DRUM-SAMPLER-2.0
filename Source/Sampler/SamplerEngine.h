#pragma once
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "../Core/MicBusLayout.h"
#include "DrumVoice.h"
#include <vector>
#include <map>
#include <array>

class SamplerEngine
{
public:
    static constexpr int MAX_VOICES = 64;
    static constexpr int NUM_DRUM_CHANNELS = MicBus::count;

    SamplerEngine();

    void loadSamplesFromFolder(const juce::File& folder);
    void prepareToPlay(double sampleRate, int blockSize);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);

    void setPitchForNote(int midiNote, float semitones);
    float getPitchForNote(int midiNote) const;
    void setVelocityCurveForNote(int midiNote, const VelocityCurve& curve);
    VelocityCurve getVelocityCurveForNote(int midiNote) const;
    void setChannelForNote(int midiNote, int channel);
    void setArticulationHintForNote(int midiNote, const juce::String& articulationKey);
    juce::String getArticulationHintForNote(int midiNote) const;
    void setDrummerProfile(const juce::String& profileKey);
    void setPlayingStyle(const juce::String& styleKey);
    juce::String getDrummerProfile() const { return currentDrummerProfile; }
    juce::String getPlayingStyle() const { return currentPlayingStyle; }

    void setMicTrim(int midiNote, int micIndex, float trim01);
    float getMicTrim(int midiNote, int micIndex) const;

    void serializeMicTrims(juce::ValueTree& state) const;
    void deserializeMicTrims(const juce::ValueTree& state);

    int getNumSamples() const { return static_cast<int>(samples.size()); }
    const juce::AudioBuffer<float>& getChannelBuffer(int channelIndex) const
    {
        return channelBuffers[juce::jlimit(0, NUM_DRUM_CHANNELS - 1, channelIndex)];
    }

    void noteOn(int midiNote, int velocity);
    void noteOff(int midiNote);

private:
    int inferMicStemFromFilename(const juce::String& filenameLowercase) const;
    int resolveOutputStem(const DrumSample& sample, int midiNote) const;
    int micTrimIndex(int midiNote, int micIndex) const;

    std::vector<DrumSample> samples;
    DrumVoice voices[MAX_VOICES];
    std::array<juce::AudioBuffer<float>, NUM_DRUM_CHANNELS> channelBuffers;

    double currentSampleRate = 44100.0;
    int rrCounters[128] = {};

    std::map<int, float> pitchSettings;
    std::map<int, VelocityCurve> velocityCurves;
    std::map<int, int> noteToChannel;
    std::map<int, juce::String> articulationHints;

    std::array<float, 128 * MicBus::count> micTrims {};
    juce::String currentDrummerProfile = "default";
    juce::String currentPlayingStyle = "auto";

    juce::AudioFormatManager formatManager;
};
