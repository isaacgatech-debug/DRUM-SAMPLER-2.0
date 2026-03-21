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
    for (auto& bus : buses)
    {
        if (bus.getBusIndex() == 0)
        {
            masterOutput.makeCopyOf(bus.getBuffer());
            bus.processAudio(masterOutput);
        }
        else
        {
            bus.processAudio(bus.getBuffer());
        }
    }
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
