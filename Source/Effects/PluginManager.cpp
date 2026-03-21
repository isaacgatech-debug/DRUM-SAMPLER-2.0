#include "PluginManager.h"

PluginManager::PluginManager()
{
    formatManager.addDefaultFormats();
    loadPluginList();
}

void PluginManager::scanForPlugins(std::function<void(const juce::String&)> progressCallback)
{
    knownPlugins.clear();
    
    for (int i = 0; i < formatManager.getNumFormats(); ++i)
    {
        auto* format = formatManager.getFormat(i);
        
        if (progressCallback)
            progressCallback("Scanning " + format->getName() + " plugins...");
        
        juce::PluginDirectoryScanner scanner(
            knownPlugins,
            *format,
            format->getDefaultLocationsToSearch(),
            true,
            juce::File()
        );
        
        juce::String pluginBeingScanned;
        while (scanner.scanNextFile(true, pluginBeingScanned))
        {
            if (progressCallback)
                progressCallback("Scanning: " + pluginBeingScanned);
        }
    }
    
    savePluginList();
}

std::vector<juce::PluginDescription> PluginManager::getAvailablePlugins() const
{
    std::vector<juce::PluginDescription> plugins;
    
    for (const auto& type : knownPlugins.getTypes())
    {
        plugins.push_back(type);
    }
    
    return plugins;
}

std::vector<juce::PluginDescription> PluginManager::searchPlugins(const juce::String& searchText) const
{
    std::vector<juce::PluginDescription> results;
    auto allPlugins = getAvailablePlugins();
    
    juce::String searchLower = searchText.toLowerCase();
    
    for (const auto& plugin : allPlugins)
    {
        if (plugin.name.toLowerCase().contains(searchLower) ||
            plugin.manufacturerName.toLowerCase().contains(searchLower) ||
            plugin.category.toLowerCase().contains(searchLower))
        {
            results.push_back(plugin);
        }
    }
    
    return results;
}

std::unique_ptr<juce::AudioPluginInstance> PluginManager::createPluginInstance(
    const juce::PluginDescription& description,
    double sampleRate,
    int blockSize,
    juce::String& errorMessage)
{
    return formatManager.createPluginInstance(description, sampleRate, blockSize, errorMessage);
}

void PluginManager::savePluginList()
{
    auto xml = knownPlugins.createXml();
    if (xml != nullptr)
    {
        xml->writeTo(getPluginListFile());
    }
}

void PluginManager::loadPluginList()
{
    auto file = getPluginListFile();
    if (file.existsAsFile())
    {
        auto xml = juce::XmlDocument::parse(file);
        if (xml != nullptr)
        {
            knownPlugins.recreateFromXml(*xml);
        }
    }
}

juce::File PluginManager::getPluginListFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("DrumSampler2")
        .getChildFile("PluginList.xml");
}
