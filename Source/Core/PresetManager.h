#pragma once
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

struct PresetData
{
    juce::String name;
    juce::String author;
    juce::String category;
    juce::String description;
    juce::File file;
    juce::ValueTree data;
    
    bool isValid() const { return file.existsAsFile() && data.isValid(); }
};

class PresetManager
{
public:
    PresetManager();
    
    void setFactoryPresetsFolder(const juce::File& folder);
    void setUserPresetsFolder(const juce::File& folder);
    
    void scanPresets();
    bool savePreset(const juce::String& name, const juce::String& category, 
                   const juce::ValueTree& state);
    bool loadPreset(const juce::String& name);
    bool deletePreset(const juce::String& name);
    
    std::vector<PresetData> getAllPresets() const { return presets; }
    std::vector<PresetData> getPresetsByCategory(const juce::String& category) const;
    std::vector<juce::String> getCategories() const;
    
    const PresetData* getCurrentPreset() const { return currentPreset; }
    juce::ValueTree getCurrentState() const;
    
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void presetLoaded(const PresetData& preset) = 0;
        virtual void presetListChanged() = 0;
    };
    
    void addListener(Listener* listener) { listeners.add(listener); }
    void removeListener(Listener* listener) { listeners.remove(listener); }
    
private:
    void loadPresetFile(const juce::File& file);
    juce::File getPresetFile(const juce::String& name) const;
    
    std::vector<PresetData> presets;
    const PresetData* currentPreset = nullptr;
    
    juce::File factoryFolder;
    juce::File userFolder;
    
    juce::ListenerList<Listener> listeners;
};
