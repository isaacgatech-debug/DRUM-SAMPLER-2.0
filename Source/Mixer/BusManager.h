#pragma once
#include "../Routing/AudioBus.h"
#include <array>

class BusManager
{
public:
    static constexpr int NUM_BUSES = 33;
    
    BusManager();
    
    void prepare(double sampleRate, int blockSize);
    void processAllBuses(juce::AudioBuffer<float>& masterOutput);
    void clearAllBuses();
    
    void configureBus(int index, AudioBus::BusType type);
    void linkBusToDAW(int busIndex, int dawOutputIndex);
    
    AudioBus& getBus(int index);
    const AudioBus& getBus(int index) const;
    
private:
    std::array<AudioBus, NUM_BUSES> buses;
    double sampleRate = 44100.0;
    int blockSize = 512;
};
