#include "AudioBus.h"

AudioBus::AudioBus(int index)
    : busIndex(index)
{
    busName = (index == 0) ? "Master" : "Bus " + juce::String(index);
}

void AudioBus::prepare(double sampleRate, int blockSize)
{
    int numChannels = (busType == BusType::Mono) ? 1 : 2;
    buffer.setSize(numChannels, blockSize);
    buffer.clear();
}

void AudioBus::processAudio(juce::AudioBuffer<float>& input)
{
    if (mute)
    {
        buffer.clear();
        return;
    }
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        buffer.applyGain(ch, 0, buffer.getNumSamples(), gain);
    }
}

void AudioBus::clear()
{
    buffer.clear();
}

void AudioBus::linkToDAW(bool shouldLink, int dawIndex)
{
    isLinkedToDAWFlag = shouldLink;
    dawOutputIndex = dawIndex;
}
