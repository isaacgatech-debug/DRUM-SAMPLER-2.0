#pragma once
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

class StateManager
{
public:
    static constexpr int kCurrentSchemaVersion = 1;
    static constexpr const char* kSchemaVersionKey = "schemaVersion";

    static void saveState(const juce::ValueTree& state, juce::MemoryBlock& destData);
    static juce::ValueTree loadState(const void* data, int sizeInBytes);
    static juce::ValueTree migrateStateIfNeeded(const juce::ValueTree& state);
};
