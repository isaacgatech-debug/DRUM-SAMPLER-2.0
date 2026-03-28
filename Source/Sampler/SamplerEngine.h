#pragma once
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "DrumVoice.h"
#include <vector>
#include <map>
#include <array>

class SamplerEngine
{
public:
    static constexpr int MAX_VOICES = 64;
    static constexpr int NUM_DRUM_CHANNELS = 11;
    
    SamplerEngine();
    
    void loadSamplesFromFolder(const juce::File& folder);
    void prepareToPlay(double sampleRate, int blockSize);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);
    
    void setPitchForNote(int midiNote, float semitones);
    void setVelocityCurveForNote(int midiNote, const VelocityCurve& curve);
    void setChannelForNote(int midiNote, int channel);
    
    int getNumSamples() const { return static_cast<int>(samples.size()); }
    const juce::AudioBuffer<float>& getChannelBuffer(int channelIndex) const
    {
        return channelBuffers[juce::jlimit(0, NUM_DRUM_CHANNELS - 1, channelIndex)];
    }
    
    void noteOn(int midiNote, int velocity);
    void noteOff(int midiNote);

private:
    std::vector<DrumSample> samples;
    DrumVoice voices[MAX_VOICES];
    std::array<juce::AudioBuffer<float>, NUM_DRUM_CHANNELS> channelBuffers;
    
    double currentSampleRate = 44100.0;
    int rrCounters[128] = {};
    
    std::map<int, float> pitchSettings;
    std::map<int, VelocityCurve> velocityCurves;
    std::map<int, int> noteToChannel;
    
    juce::AudioFormatManager formatManager;
};
