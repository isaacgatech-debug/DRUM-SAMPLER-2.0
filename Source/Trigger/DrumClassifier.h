#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "OnsetDetector.h"

enum class DrumType
{
    Kick,
    Snare,
    HiHat,
    Tom,
    Crash,
    Ride,
    Unknown
};

struct SpectralFeatures
{
    float centroid;
    float brightness;
    float lowEnergy;
    float midEnergy;
    float highEnergy;
    float zeroCrossingRate;
};

class DrumClassifier
{
public:
    DrumClassifier();
    
    void prepare(double sampleRate);
    DrumType classify(const juce::AudioBuffer<float>& audioSegment, const OnsetEvent& onset);
    
    int getDrumTypeToMIDINote(DrumType type) const;
    juce::String getDrumTypeName(DrumType type) const;
    
private:
    SpectralFeatures extractFeatures(const juce::AudioBuffer<float>& segment);
    DrumType classifyFromFeatures(const SpectralFeatures& features);
    
    juce::dsp::FFT fft;
    std::vector<float> fftData;
    double sampleRate = 44100.0;
};
