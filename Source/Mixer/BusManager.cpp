#include "BusManager.h"

BusManager::BusManager()
{
    for (int i = 0; i < NUM_BUSES; ++i)
    {
        buses[i] = AudioBus(i);
    }
}

void BusManager::prepare(double sr, int bs)
{
    sampleRate = sr;
    blockSize = bs;
    
    for (auto& bus : buses)
    {
        bus.prepare(sr, bs);
    }
}

void BusManager::processAllBuses(juce::AudioBuffer<float>& masterOutput)
{
    auto& masterBus = buses[0];
    masterBus.clear();

    auto& masterBuffer = masterBus.getBuffer();
    for (int busIndex = 1; busIndex < NUM_BUSES; ++busIndex)
    {
        auto& bus = buses[busIndex];
        bus.processAudio(bus.getBuffer());
        auto& busBuffer = bus.getBuffer();

        const int channels = juce::jmin(masterBuffer.getNumChannels(), busBuffer.getNumChannels());
        const int samples = juce::jmin(masterBuffer.getNumSamples(), busBuffer.getNumSamples());
        for (int ch = 0; ch < channels; ++ch)
            masterBuffer.addFrom(ch, 0, busBuffer, ch, 0, samples);
    }

    masterBus.processAudio(masterBuffer);
    masterOutput.makeCopyOf(masterBus.getBuffer(), true);
}

void BusManager::clearAllBuses()
{
    for (auto& bus : buses)
    {
        bus.clear();
    }
}

void BusManager::configureBus(int index, AudioBus::BusType type)
{
    if (index >= 0 && index < NUM_BUSES)
    {
        buses[index].setType(type);
    }
}

void BusManager::linkBusToDAW(int busIndex, int dawOutputIndex)
{
    if (busIndex >= 0 && busIndex < NUM_BUSES)
    {
        buses[busIndex].linkToDAW(true, dawOutputIndex);
    }
}

AudioBus& BusManager::getBus(int index)
{
    return buses[juce::jlimit(0, NUM_BUSES - 1, index)];
}

const AudioBus& BusManager::getBus(int index) const
{
    return buses[juce::jlimit(0, NUM_BUSES - 1, index)];
}
