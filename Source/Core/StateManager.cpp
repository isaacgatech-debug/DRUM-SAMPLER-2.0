#include "StateManager.h"

void StateManager::saveState(const juce::ValueTree& state, juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    if (auto xml = state.createXml())
        xml->writeTo(stream);
}

juce::ValueTree StateManager::loadState(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    auto xml = juce::parseXML(stream.readEntireStreamAsString());
    if (xml == nullptr)
        return {};

    return juce::ValueTree::fromXml(*xml);
}

juce::ValueTree StateManager::migrateStateIfNeeded(const juce::ValueTree& state)
{
    if (!state.isValid())
        return state;

    auto migrated = state.createCopy();
    auto schemaVersion = static_cast<int>(migrated.getProperty(kSchemaVersionKey, 0));
    if (schemaVersion < kCurrentSchemaVersion)
        migrated.setProperty(kSchemaVersionKey, kCurrentSchemaVersion, nullptr);
    return migrated;
}
