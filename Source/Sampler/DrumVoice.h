#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

struct DrumSample
{
    juce::AudioBuffer<float> buffer;
    int sampleRate = 44100;
    int midiNote = 0;
    int rrGroup = 0;
    int velLow = 0;
    int velHigh = 127;
};

struct VelocityCurve
{
    float attack = 5.0f;
    float decay = 150.0f;
    float sustain = 0.8f;
    float release = 200.0f;
    
    enum class CurveType {
        Linear,
        Exponential,
        Logarithmic,
        SCurve
    };
    CurveType responseType = CurveType::Linear;
};

class DrumVoice
{
public:
    DrumVoice();
    
    void trigger(const DrumSample* sample, float gain, int channel = 0);
    void process(juce::AudioBuffer<float>& output, int startSample, int numSamples);
    void setPitch(float semitones);
    void setVelocityCurve(const VelocityCurve& curve);
    void setOutputChannel(int channel) { outputChannel = channel; }
    int getOutputChannel() const { return outputChannel; }
    void setSampleRate(double sampleRate) { adsr.setSampleRate(sampleRate); }
    
    bool isActive() const { return active; }
    void stop();
    
private:
    const DrumSample* currentSample = nullptr;
    int position = 0;
    float gain = 1.0f;
    bool active = false;
    int outputChannel = 0;
    
    float pitchRatio = 1.0f;
    VelocityCurve velocityCurve;
    juce::ADSR adsr;
};
