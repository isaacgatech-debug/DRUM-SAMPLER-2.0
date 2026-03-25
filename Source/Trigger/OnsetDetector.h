#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>

// Forward declaration
enum class DrumType;

struct OnsetEvent
{
    double timeInSamples;
    float strength;
    int channel;
};

struct FrequencyBand
{
    juce::String name;
    float minFreq;
    float maxFreq;
    float threshold;
    float sensitivity;
    bool enabled;
    float currentEnergy;
    float smoothedEnergy;
    
    FrequencyBand(const juce::String& n, float minF, float maxF)
        : name(n), minFreq(minF), maxFreq(maxF), 
          threshold(0.3f), sensitivity(1.0f), enabled(true),
          currentEnergy(0.0f), smoothedEnergy(0.0f) {}
};

class OnsetDetector
{
public:
    OnsetDetector();
    
    void prepare(double sampleRate, int blockSize);
    void processBlock(const juce::AudioBuffer<float>& buffer, std::vector<OnsetEvent>& onsets);
    
    void setThreshold(float threshold);
    void setMinTimeBetweenOnsets(double timeMs) { minOnsetGap = timeMs; }
    void setSensitivity(float sens) { globalSensitivity = juce::jlimit(0.1f, 10.0f, sens); }
    
    void setBandThreshold(int bandIndex, float threshold);
    void setBandEnabled(int bandIndex, bool enabled);
    void setBandSensitivity(int bandIndex, float sensitivity);
    
    float getThreshold() const { return globalThreshold; }
    float getSensitivity() const { return globalSensitivity; }
    std::vector<FrequencyBand>& getBands() { return bands; }
    const std::vector<FrequencyBand>& getBands() const { return bands; }
    
    void setTargetDrumType(DrumType type) { targetDrumType = type; }
    
private:
    void calculateBandEnergies(const std::vector<float>& spectrum);
    void detectOnsetsFromBands(int samplePosition, std::vector<OnsetEvent>& onsets, int channel);
    int freqToBin(float frequency) const;
    
    juce::dsp::FFT fft;
    std::vector<float> fftData;
    std::vector<FrequencyBand> bands;
    
    float globalThreshold = 0.3f;
    float globalSensitivity = 1.0f;
    double minOnsetGap = 50.0;
    
    double sampleRate = 44100.0;
    int fftSize = 2048;
    int hopSize = 512;
    
    double lastOnsetTime = 0.0;
    double currentSamplePosition = 0.0;
    
    DrumType targetDrumType;
};
