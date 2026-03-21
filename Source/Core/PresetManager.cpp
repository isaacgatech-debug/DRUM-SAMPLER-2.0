#include "PresetManager.h"
#include "ErrorLogger.h"

PresetManager::PresetManager()
{
    userFolder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("DrumSampler2")
        .getChildFile("Presets");
    
    userFolder.createDirectory();
}

void PresetManager::setFactoryPresetsFolder(const juce::File& folder)
{
    factoryFolder = folder;
}

void PresetManager::setUserPresetsFolder(const juce::File& folder)
{
    userFolder = folder;
    userFolder.createDirectory();
}

void PresetManager::scanPresets()
{
    presets.clear();
    
    auto scanFolder = [this](const juce::File& folder, bool isFactory)
    {
        if (!folder.exists() || !folder.isDirectory())
            return;
        
        auto files = folder.findChildFiles(juce::File::findFiles, true, "*.ds2preset");
        
        for (const auto& file : files)
        {
            loadPresetFile(file);
        }
    };
    
    if (factoryFolder.exists())
        scanFolder(factoryFolder, true);
    
    if (userFolder.exists())
        scanFolder(userFolder, false);
    
    LOG_INFO("Scanned " + juce::String(presets.size()) + " presets");
    listeners.call([](Listener& l) { l.presetListChanged(); });
}

bool PresetManager::savePreset(const juce::String& name, const juce::String& category,
                               const juce::ValueTree& state)
{
    auto categoryFolder = userFolder.getChildFile(category);
    categoryFolder.createDirectory();
    
    auto file = categoryFolder.getChildFile(name + ".ds2preset");
    
    juce::ValueTree preset("Preset");
    preset.setProperty("name", name, nullptr);
    preset.setProperty("category", category, nullptr);
    preset.setProperty("author", juce::SystemStats::getFullUserName(), nullptr);
    preset.setProperty("date", juce::Time::getCurrentTime().toString(true, true), nullptr);
    preset.appendChild(state.createCopy(), nullptr);
    
    std::unique_ptr<juce::XmlElement> xml(preset.createXml());
    if (xml && xml->writeTo(file))
    {
        LOG_INFO("Saved preset: " + name);
        scanPresets();
        return true;
    }
    
    LOG_ERROR("Failed to save preset: " + name);
    return false;
}

bool PresetManager::loadPreset(const juce::String& name)
{
    for (const auto& preset : presets)
    {
        if (preset.name == name)
        {
            currentPreset = &preset;
            LOG_INFO("Loaded preset: " + name);
            listeners.call([&preset](Listener& l) { l.presetLoaded(preset); });
            return true;
        }
    }
    
    LOG_ERROR("Preset not found: " + name);
    return false;
}

bool PresetManager::deletePreset(const juce::String& name)
{
    auto file = getPresetFile(name);
    
    if (file.existsAsFile() && file.deleteFile())
    {
        LOG_INFO("Deleted preset: " + name);
        scanPresets();
        return true;
    }
    
    LOG_ERROR("Failed to delete preset: " + name);
    return false;
}

std::vector<PresetData> PresetManager::getPresetsByCategory(const juce::String& category) const
{
    std::vector<PresetData> filtered;
    
    for (const auto& preset : presets)
    {
        if (preset.category == category)
            filtered.push_back(preset);
    }
    
    return filtered;
}

std::vector<juce::String> PresetManager::getCategories() const
{
    std::vector<juce::String> categories;
    
    for (const auto& preset : presets)
    {
        if (std::find(categories.begin(), categories.end(), preset.category) == categories.end())
        {
            categories.push_back(preset.category);
        }
    }
    
    std::sort(categories.begin(), categories.end());
    return categories;
}

juce::ValueTree PresetManager::getCurrentState() const
{
    if (currentPreset && currentPreset->data.isValid())
    {
        return currentPreset->data.getChild(0);
    }
    
    return juce::ValueTree();
}

void PresetManager::loadPresetFile(const juce::File& file)
{
    std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(file));
    
    if (xml)
    {
        auto preset = juce::ValueTree::fromXml(*xml);
        
        if (preset.isValid())
        {
            PresetData data;
            data.name = preset.getProperty("name", file.getFileNameWithoutExtension());
            data.category = preset.getProperty("category", "Uncategorized");
            data.author = preset.getProperty("author", "Unknown");
            data.description = preset.getProperty("description", "");
            data.file = file;
            data.data = preset;
            
            presets.push_back(data);
        }
    }
}

juce::File PresetManager::getPresetFile(const juce::String& name) const
{
    for (const auto& preset : presets)
    {
        if (preset.name == name)
            return preset.file;
    }
    
    return juce::File();
}
