#pragma once
#include <juce_core/juce_core.h>

class StateManager
{
public:
    StateManager();
    
    void saveState(juce::MemoryBlock& destData);
    void loadState(const void* data, int sizeInBytes);
    
private:
};
