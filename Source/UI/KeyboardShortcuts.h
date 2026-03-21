#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <map>
#include <functional>

enum class ShortcutAction
{
    // Playback
    PlayStop,
    Record,
    
    // Navigation
    NextTab,
    PreviousTab,
    KitTab,
    GroovesTab,
    MixerTab,
    TriggerTab,
    RoutingTab,
    
    // File Operations
    SavePreset,
    LoadPreset,
    ExportMIDI,
    ImportAudio,
    
    // Editing
    Undo,
    Redo,
    Copy,
    Paste,
    Delete,
    
    // View
    ToggleDebugConsole,
    ZoomIn,
    ZoomOut,
    
    // Mixer
    MuteAll,
    SoloAll,
    ResetMixer,
    
    // Other
    ShowHelp,
    Preferences
};

struct KeyboardShortcut
{
    juce::KeyPress keyPress;
    ShortcutAction action;
    juce::String description;
};

class KeyboardShortcutManager : public juce::KeyListener
{
public:
    static KeyboardShortcutManager& getInstance()
    {
        static KeyboardShortcutManager instance;
        return instance;
    }
    
    void registerCallback(ShortcutAction action, std::function<void()> callback)
    {
        callbacks[action] = callback;
    }
    
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override
    {
        for (const auto& shortcut : shortcuts)
        {
            if (shortcut.keyPress == key)
            {
                auto it = callbacks.find(shortcut.action);
                if (it != callbacks.end() && it->second)
                {
                    it->second();
                    return true;
                }
            }
        }
        return false;
    }
    
    std::vector<KeyboardShortcut> getAllShortcuts() const
    {
        return shortcuts;
    }
    
    juce::String getShortcutDescription(ShortcutAction action) const
    {
        for (const auto& shortcut : shortcuts)
        {
            if (shortcut.action == action)
                return shortcut.description;
        }
        return "";
    }
    
private:
    KeyboardShortcutManager()
    {
        initializeDefaultShortcuts();
    }
    
    void initializeDefaultShortcuts()
    {
        // Playback
        shortcuts.push_back({juce::KeyPress::spaceKey, ShortcutAction::PlayStop, "Play/Stop"});
        shortcuts.push_back({juce::KeyPress('r', juce::ModifierKeys::commandModifier, 0), ShortcutAction::Record, "Record"});
        
        // Navigation
        shortcuts.push_back({juce::KeyPress::tabKey, ShortcutAction::NextTab, "Next Tab"});
        shortcuts.push_back({juce::KeyPress::tabKey, juce::ModifierKeys::shiftModifier, 0), ShortcutAction::PreviousTab, "Previous Tab"});
        shortcuts.push_back({juce::KeyPress('1', juce::ModifierKeys::commandModifier, 0), ShortcutAction::KitTab, "Kit Tab"});
        shortcuts.push_back({juce::KeyPress('2', juce::ModifierKeys::commandModifier, 0), ShortcutAction::GroovesTab, "Grooves Tab"});
        shortcuts.push_back({juce::KeyPress('3', juce::ModifierKeys::commandModifier, 0), ShortcutAction::MixerTab, "Mixer Tab"});
        shortcuts.push_back({juce::KeyPress('4', juce::ModifierKeys::commandModifier, 0), ShortcutAction::TriggerTab, "Trigger Tab"});
        shortcuts.push_back({juce::KeyPress('5', juce::ModifierKeys::commandModifier, 0), ShortcutAction::RoutingTab, "Routing Tab"});
        
        // File Operations
        shortcuts.push_back({juce::KeyPress('s', juce::ModifierKeys::commandModifier, 0), ShortcutAction::SavePreset, "Save Preset"});
        shortcuts.push_back({juce::KeyPress('o', juce::ModifierKeys::commandModifier, 0), ShortcutAction::LoadPreset, "Load Preset"});
        shortcuts.push_back({juce::KeyPress('e', juce::ModifierKeys::commandModifier, 0), ShortcutAction::ExportMIDI, "Export MIDI"});
        shortcuts.push_back({juce::KeyPress('i', juce::ModifierKeys::commandModifier, 0), ShortcutAction::ImportAudio, "Import Audio"});
        
        // Editing
        shortcuts.push_back({juce::KeyPress('z', juce::ModifierKeys::commandModifier, 0), ShortcutAction::Undo, "Undo"});
        shortcuts.push_back({juce::KeyPress('z', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier, 0), ShortcutAction::Redo, "Redo"});
        shortcuts.push_back({juce::KeyPress('c', juce::ModifierKeys::commandModifier, 0), ShortcutAction::Copy, "Copy"});
        shortcuts.push_back({juce::KeyPress('v', juce::ModifierKeys::commandModifier, 0), ShortcutAction::Paste, "Paste"});
        shortcuts.push_back({juce::KeyPress::deleteKey, ShortcutAction::Delete, "Delete"});
        
        // View
        shortcuts.push_back({juce::KeyPress('d', juce::ModifierKeys::commandModifier, 0), ShortcutAction::ToggleDebugConsole, "Toggle Debug Console"});
        shortcuts.push_back({juce::KeyPress('=', juce::ModifierKeys::commandModifier, 0), ShortcutAction::ZoomIn, "Zoom In"});
        shortcuts.push_back({juce::KeyPress('-', juce::ModifierKeys::commandModifier, 0), ShortcutAction::ZoomOut, "Zoom Out"});
        
        // Mixer
        shortcuts.push_back({juce::KeyPress('m', juce::ModifierKeys::commandModifier, 0), ShortcutAction::MuteAll, "Mute All"});
        shortcuts.push_back({juce::KeyPress('s', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier, 0), ShortcutAction::SoloAll, "Solo All"});
        shortcuts.push_back({juce::KeyPress('r', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier, 0), ShortcutAction::ResetMixer, "Reset Mixer"});
        
        // Other
        shortcuts.push_back({juce::KeyPress::F1Key, ShortcutAction::ShowHelp, "Show Help"});
        shortcuts.push_back({juce::KeyPress(',', juce::ModifierKeys::commandModifier, 0), ShortcutAction::Preferences, "Preferences"});
    }
    
    std::vector<KeyboardShortcut> shortcuts;
    std::map<ShortcutAction, std::function<void()>> callbacks;
};
