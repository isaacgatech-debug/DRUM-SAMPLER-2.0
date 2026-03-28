#include "OnsetDetector.h"
#include "DrumClassifier.h"
#include <cmath>

OnsetDetector::OnsetDetector()
    : fft(11), targetDrumType(DrumType::Unknown)
{
    fftSize = 2048;
    hopSize = 512;
    
    // Initialize default frequency bands for multiband detection
    bands.emplace_back("Sub/Low", 20.0f, 60.0f);      // Sub bass / kick fundamental
    bands.emplace_back("Low", 60.0f, 200.0f);         // Kick body
    bands.emplace_back("Low-Mid", 200.0f, 500.0f);      // Snare body, tom fundamental
    bands.emplace_back("Mid", 500.0f, 2000.0f);       // Snare crack, presence
    bands.emplace_back("High-Mid", 2000.0f, 5000.0f);  // Hihat, cymbal body
    bands.emplace_back("High", 5000.0f, 12000.0f);     // Hihat, cymbal shimmer
    bands.emplace_back("Air", 12000.0f, 20000.0f);     // Ultra high frequencies
}

void OnsetDetector::prepare(double sr, int blockSize)
{
    sampleRate = sr;
    fftData.resize(fftSize * 2, 0.0f);
}

void OnsetDetector::processBlock(const juce::AudioBuffer<float>& buffer, std::vector<OnsetEvent>& onsets)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples <= 0 || numChannels <= 0)
        return;

    std::vector<float> mono(static_cast<size_t>(numSamples), 0.0f);
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* channelData = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            mono[static_cast<size_t>(i)] += channelData[i];
    }
    const float channelScale = 1.0f / static_cast<float>(numChannels);
    for (auto& sample : mono)
        sample *= channelScale;

    for (int i = 0; i < numSamples; i += hopSize)
    {
        int samplesToProcess = juce::jmin(hopSize, numSamples - i);
        
        // Fill FFT buffer with windowed data
        std::fill(fftData.begin(), fftData.end(), 0.0f);
        
        for (int j = 0; j < samplesToProcess && j < fftSize; ++j)
        {
            float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * j / fftSize));
            fftData[j] = mono[static_cast<size_t>(i + j)] * window;
        }
        
        // Perform FFT
        fft.performFrequencyOnlyForwardTransform(fftData.data());
        
        // Convert to magnitude spectrum
        std::vector<float> spectrum(fftSize / 2);
        for (int j = 0; j < fftSize / 2; ++j)
        {
            float real = fftData[j * 2];
            float imag = fftData[j * 2 + 1];
            spectrum[j] = std::sqrt(real * real + imag * imag);
        }
        
        // Calculate energies in each frequency band
        calculateBandEnergies(spectrum);
        
        // Detect onsets based on band energies
        detectOnsetsFromBands(static_cast<int>(currentSamplePosition) + i, onsets, 0);
    }

    currentSamplePosition += numSamples;
}

void OnsetDetector::calculateBandEnergies(const std::vector<float>& spectrum)
{
    for (auto& band : bands)
    {
        if (!band.enabled)
        {
            band.currentEnergy = 0.0f;
            continue;
        }
        
        int minBin = freqToBin(band.minFreq);
        int maxBin = freqToBin(band.maxFreq);
        
        float energy = 0.0f;
        for (int bin = minBin; bin < maxBin && bin < static_cast<int>(spectrum.size()); ++bin)
        {
            energy += spectrum[bin] * spectrum[bin];
        }
        
        // Convert to RMS-like value
        energy = std::sqrt(energy / (maxBin - minBin));
        
        // Smooth the energy (simple exponential smoothing)
        const float smoothing = 0.7f;
        band.currentEnergy = smoothing * band.smoothedEnergy + (1.0f - smoothing) * energy;
        band.smoothedEnergy = band.currentEnergy;
    }
}

void OnsetDetector::detectOnsetsFromBands(int samplePosition, std::vector<OnsetEvent>& onsets, int channel)
{
    // Calculate weighted onset strength based on enabled bands
    float onsetStrength = 0.0f;
    int activeBands = 0;
    
    // Configure which bands to prioritize based on target drum type
    std::vector<int> priorityBands;
    
    switch (targetDrumType)
    {
        case DrumType::Kick:
            // Prioritize sub and low bands
            priorityBands = {0, 1};  // Sub/Low and Low bands
            break;
        case DrumType::Snare:
            // Prioritize low-mid and mid bands
            priorityBands = {2, 3};  // Low-Mid and Mid bands
            break;
        case DrumType::HiHat:
        case DrumType::Crash:
        case DrumType::Ride:
            // Prioritize high-mid and high bands
            priorityBands = {4, 5, 6};  // High-Mid, High, and Air bands
            break;
        case DrumType::Tom:
            // Prioritize low-mid band
            priorityBands = {2};  // Low-Mid band
            break;
        default:
            // Auto-detect: use all enabled bands
            break;
    }
    
    // If we have priority bands for a specific drum type, only use those
    if (!priorityBands.empty())
    {
        for (int bandIdx : priorityBands)
        {
            if (bandIdx < static_cast<int>(bands.size()) && bands[bandIdx].enabled)
            {
                float bandThreshold = bands[bandIdx].threshold / bands[bandIdx].sensitivity;
                
                if (bands[bandIdx].currentEnergy > bandThreshold)
                {
                    onsetStrength += bands[bandIdx].currentEnergy * bands[bandIdx].sensitivity;
                    activeBands++;
                }
            }
        }
    }
    else
    {
        // Auto-detect: use all enabled bands
        for (const auto& band : bands)
        {
            if (band.enabled && band.currentEnergy > band.threshold)
            {
                onsetStrength += band.currentEnergy * band.sensitivity;
                activeBands++;
            }
        }
    }
    
    // Normalize by number of active bands
    if (activeBands > 0)
    {
        onsetStrength /= activeBands;
    }
    
    // Apply global sensitivity
    onsetStrength *= globalSensitivity;
    
    // Check if we have an onset
    if (onsetStrength > globalThreshold)
    {
        double timeSinceLastOnset = (samplePosition - lastOnsetTime) / sampleRate * 1000.0;
        
        if (timeSinceLastOnset > minOnsetGap)
        {
            OnsetEvent event;
            event.timeInSamples = samplePosition;
            event.strength = onsetStrength;
            event.channel = channel;
            onsets.push_back(event);
            
            lastOnsetTime = samplePosition;
        }
    }
}

int OnsetDetector::freqToBin(float frequency) const
{
    return static_cast<int>(frequency * fftSize / static_cast<float>(sampleRate));
}

void OnsetDetector::setThreshold(float threshold)
{
    globalThreshold = juce::jlimit(0.0f, 1.0f, threshold);
}

void OnsetDetector::setBandThreshold(int bandIndex, float threshold)
{
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bands.size()))
    {
        bands[bandIndex].threshold = juce::jlimit(0.0f, 1.0f, threshold);
    }
}

void OnsetDetector::setBandEnabled(int bandIndex, bool enabled)
{
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bands.size()))
    {
        bands[bandIndex].enabled = enabled;
    }
}

void OnsetDetector::setBandSensitivity(int bandIndex, float sensitivity)
{
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bands.size()))
    {
        bands[bandIndex].sensitivity = juce::jlimit(0.1f, 10.0f, sensitivity);
    }
}
