#include "OnsetDetector.h"
#include <cmath>

OnsetDetector::OnsetDetector()
    : fft(11)
{
    fftSize = 2048;
    hopSize = 512;
}

void OnsetDetector::prepare(double sr, int blockSize)
{
    sampleRate = sr;
    
    fftData.resize(fftSize * 2, 0.0f);
    previousSpectrum.resize(fftSize / 2, 0.0f);
    currentSpectrum.resize(fftSize / 2, 0.0f);
    envelope.resize(1024, 0.0f);
    envelopePos = 0;
}

void OnsetDetector::processBlock(const juce::AudioBuffer<float>& buffer, std::vector<OnsetEvent>& onsets)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* channelData = buffer.getReadPointer(ch);
        
        for (int i = 0; i < numSamples; i += hopSize)
        {
            int samplesToProcess = juce::jmin(hopSize, numSamples - i);
            
            std::fill(fftData.begin(), fftData.end(), 0.0f);
            
            for (int j = 0; j < samplesToProcess && j < fftSize; ++j)
            {
                float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * j / fftSize));
                fftData[j] = channelData[i + j] * window;
            }
            
            fft.performFrequencyOnlyForwardTransform(fftData.data());
            
            for (int j = 0; j < fftSize / 2; ++j)
            {
                currentSpectrum[j] = std::sqrt(fftData[j * 2] * fftData[j * 2] + 
                                               fftData[j * 2 + 1] * fftData[j * 2 + 1]);
            }
            
            float flux = calculateSpectralFlux(currentSpectrum, previousSpectrum);
            
            updatePeakDetection(flux * sensitivity, 
                              static_cast<int>(currentSamplePosition) + i, 
                              onsets, ch);
            
            previousSpectrum = currentSpectrum;
        }
        
        currentSamplePosition += numSamples;
    }
}

float OnsetDetector::calculateSpectralFlux(const std::vector<float>& current,
                                          const std::vector<float>& previous)
{
    float flux = 0.0f;
    
    for (size_t i = 0; i < current.size(); ++i)
    {
        float diff = current[i] - previous[i];
        if (diff > 0.0f)
            flux += diff;
    }
    
    return flux / static_cast<float>(current.size());
}

void OnsetDetector::updatePeakDetection(float fluxValue, int samplePosition,
                                       std::vector<OnsetEvent>& onsets, int channel)
{
    envelope[envelopePos % envelope.size()] = fluxValue;
    envelopePos++;
    
    if (fluxValue > detectionThreshold)
    {
        double timeSinceLastOnset = (samplePosition - lastOnsetTime) / sampleRate * 1000.0;
        
        if (timeSinceLastOnset > minOnsetGap)
        {
            bool isPeak = true;
            int windowSize = 5;
            
            for (int i = 1; i <= windowSize; ++i)
            {
                int prevIdx = (envelopePos - i + envelope.size()) % envelope.size();
                if (envelope[prevIdx] > fluxValue)
                {
                    isPeak = false;
                    break;
                }
            }
            
            if (isPeak)
            {
                OnsetEvent onset;
                onset.timeInSamples = samplePosition;
                onset.strength = fluxValue;
                onset.channel = channel;
                onsets.push_back(onset);
                
                lastOnsetTime = samplePosition;
            }
        }
    }
}
