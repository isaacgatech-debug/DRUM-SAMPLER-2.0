#include "DrumClassifier.h"
#include <cmath>

DrumClassifier::DrumClassifier()
    : fft(11)
{
    fftData.resize(4096, 0.0f);
}

void DrumClassifier::prepare(double sr)
{
    sampleRate = sr;
}

DrumType DrumClassifier::classify(const juce::AudioBuffer<float>& audioSegment, const OnsetEvent& onset)
{
    auto features = extractFeatures(audioSegment);
    return classifyFromFeatures(features);
}

SpectralFeatures DrumClassifier::extractFeatures(const juce::AudioBuffer<float>& segment)
{
    SpectralFeatures features;
    
    const int numSamples = juce::jmin(2048, segment.getNumSamples());
    const float* data = segment.getReadPointer(0);
    
    std::fill(fftData.begin(), fftData.end(), 0.0f);
    
    for (int i = 0; i < numSamples; ++i)
    {
        float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / numSamples));
        fftData[i] = data[i] * window;
    }
    
    fft.performFrequencyOnlyForwardTransform(fftData.data());
    
    float totalEnergy = 0.0f;
    float weightedSum = 0.0f;
    features.lowEnergy = 0.0f;
    features.midEnergy = 0.0f;
    features.highEnergy = 0.0f;
    
    int lowBand = 200;
    int midBand = 2000;
    int highBand = 8000;
    
    for (int i = 0; i < 1024; ++i)
    {
        float magnitude = std::sqrt(fftData[i * 2] * fftData[i * 2] + 
                                    fftData[i * 2 + 1] * fftData[i * 2 + 1]);
        
        float frequency = (i * sampleRate) / 2048.0f;
        
        totalEnergy += magnitude;
        weightedSum += magnitude * frequency;
        
        if (frequency < lowBand)
            features.lowEnergy += magnitude;
        else if (frequency < midBand)
            features.midEnergy += magnitude;
        else if (frequency < highBand)
            features.highEnergy += magnitude;
    }
    
    features.centroid = (totalEnergy > 0.0f) ? (weightedSum / totalEnergy) : 0.0f;
    
    if (totalEnergy > 0.0f)
    {
        features.lowEnergy /= totalEnergy;
        features.midEnergy /= totalEnergy;
        features.highEnergy /= totalEnergy;
    }
    
    features.brightness = features.highEnergy / (features.lowEnergy + features.midEnergy + features.highEnergy + 0.001f);
    
    int zeroCrossings = 0;
    for (int i = 1; i < numSamples; ++i)
    {
        if ((data[i] >= 0.0f && data[i - 1] < 0.0f) ||
            (data[i] < 0.0f && data[i - 1] >= 0.0f))
        {
            zeroCrossings++;
        }
    }
    features.zeroCrossingRate = static_cast<float>(zeroCrossings) / numSamples;
    
    return features;
}

DrumType DrumClassifier::classifyFromFeatures(const SpectralFeatures& features)
{
    if (features.lowEnergy > 0.6f && features.centroid < 200.0f)
    {
        return DrumType::Kick;
    }
    
    if (features.brightness > 0.5f && features.zeroCrossingRate > 0.3f)
    {
        return DrumType::HiHat;
    }
    
    if (features.midEnergy > 0.4f && features.brightness > 0.3f && features.brightness < 0.6f)
    {
        return DrumType::Snare;
    }
    
    if (features.brightness > 0.4f && features.highEnergy > 0.3f)
    {
        return DrumType::Crash;
    }
    
    if (features.midEnergy > 0.5f && features.lowEnergy > 0.2f)
    {
        return DrumType::Tom;
    }
    
    if (features.brightness > 0.35f && features.midEnergy > 0.3f)
    {
        return DrumType::Ride;
    }
    
    return DrumType::Unknown;
}

int DrumClassifier::getDrumTypeToMIDINote(DrumType type) const
{
    switch (type)
    {
        case DrumType::Kick:   return 36;
        case DrumType::Snare:  return 38;
        case DrumType::HiHat:  return 42;
        case DrumType::Tom:    return 48;
        case DrumType::Crash:  return 49;
        case DrumType::Ride:   return 51;
        default:               return 60;
    }
}

juce::String DrumClassifier::getDrumTypeName(DrumType type) const
{
    switch (type)
    {
        case DrumType::Kick:   return "Kick";
        case DrumType::Snare:  return "Snare";
        case DrumType::HiHat:  return "HiHat";
        case DrumType::Tom:    return "Tom";
        case DrumType::Crash:  return "Crash";
        case DrumType::Ride:   return "Ride";
        default:               return "Unknown";
    }
}
