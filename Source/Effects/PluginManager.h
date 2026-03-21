#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

class PluginManager
{
public:
    PluginManager();
    
    void scanForPlugins(std::function<void(const juce::String&)> progressCallback = nullptr);
    std::vector<juce::PluginDescription> getAvailablePlugins() const;
    std::vector<juce::PluginDescription> searchPlugins(const juce::String& searchText) const;
    
    std::unique_ptr<juce::AudioPluginInstance> createPluginInstance(
        const juce::PluginDescription& description,
        double sampleRate,
        int blockSize,
        juce::String& errorMessage
    );
    
    juce::KnownPluginList& getPluginList() { return knownPlugins; }
    juce::AudioPluginFormatManager& getFormatManager() { return formatManager; }
    
    void savePluginList();
    void loadPluginList();
    
private:
    juce::File getPluginListFile() const;
    
    juce::AudioPluginFormatManager formatManager;
    juce::KnownPluginList knownPlugins;
};
