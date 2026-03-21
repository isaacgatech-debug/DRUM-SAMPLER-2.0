#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>

struct OnsetEvent
{
    double timeInSamples;
    float strength;
    int channel;
};

class OnsetDetector
{
public:
    OnsetDetector();
    
    void prepare(double sampleRate, int blockSize);
    void processBlock(const juce::AudioBuffer<float>& buffer, std::vector<OnsetEvent>& onsets);
    
    void setThreshold(float threshold) { detectionThreshold = juce::jlimit(0.0f, 1.0f, threshold); }
    void setMinTimeBetweenOnsets(double timeMs) { minOnsetGap = timeMs; }
    void setSensitivity(float sens) { sensitivity = juce::jlimit(0.1f, 10.0f, sens); }
    
    float getThreshold() const { return detectionThreshold; }
    float getSensitivity() const { return sensitivity; }
    
private:
    float calculateSpectralFlux(const std::vector<float>& currentSpectrum,
                                const std::vector<float>& previousSpectrum);
    void updatePeakDetection(float fluxValue, int samplePosition, 
                            std::vector<OnsetEvent>& onsets, int channel);
    
    juce::dsp::FFT fft;
    std::vector<float> fftData;
    std::vector<float> previousSpectrum;
    std::vector<float> currentSpectrum;
    
    float detectionThreshold = 0.3f;
    float sensitivity = 1.0f;
    double minOnsetGap = 50.0;
    
    double sampleRate = 44100.0;
    int fftSize = 2048;
    int hopSize = 512;
    
    double lastOnsetTime = 0.0;
    double currentSamplePosition = 0.0;
    
    std::vector<float> envelope;
    int envelopePos = 0;
};
