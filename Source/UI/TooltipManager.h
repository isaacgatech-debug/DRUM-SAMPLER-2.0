#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <map>

class TooltipManager
{
public:
    static TooltipManager& getInstance()
    {
        static TooltipManager instance;
        return instance;
    }
    
    void registerTooltip(const juce::String& componentId, const juce::String& tooltip)
    {
        tooltips[componentId] = tooltip;
    }
    
    juce::String getTooltip(const juce::String& componentId) const
    {
        auto it = tooltips.find(componentId);
        return (it != tooltips.end()) ? it->second : juce::String();
    }
    
    void initializeDefaultTooltips()
    {
        // Sampler
        registerTooltip("pad_kick", "Kick Drum - MIDI Note C1 (36)");
        registerTooltip("pad_snare", "Snare Drum - MIDI Note D1 (38)");
        registerTooltip("pad_hihat", "Hi-Hat - MIDI Note F#1 (42)");
        registerTooltip("pad_tom1", "Tom 1 - MIDI Note A1 (45)");
        registerTooltip("pad_tom2", "Tom 2 - MIDI Note C2 (48)");
        registerTooltip("pad_crash", "Crash Cymbal - MIDI Note C#2 (49)");
        registerTooltip("pad_ride", "Ride Cymbal - MIDI Note D#2 (51)");
        
        // Mixer
        registerTooltip("mixer_volume", "Channel Volume (dB)");
        registerTooltip("mixer_pan", "Stereo Pan Position");
        registerTooltip("mixer_mute", "Mute Channel");
        registerTooltip("mixer_solo", "Solo Channel");
        registerTooltip("mixer_phase", "Invert Phase");
        
        // Effects
        registerTooltip("fx_reverb", "Add Reverb Effect");
        registerTooltip("fx_delay", "Add Delay Effect");
        registerTooltip("fx_eq", "Add EQ Effect");
        registerTooltip("fx_compressor", "Add Compressor Effect");
        registerTooltip("fx_transient", "Add Transient Shaper Effect");
        
        // Trigger
        registerTooltip("trigger_threshold", "Detection Threshold (0.0-1.0)");
        registerTooltip("trigger_bleed", "Bleed Suppression Amount");
        registerTooltip("trigger_process", "Process Audio File");
        registerTooltip("trigger_export", "Export to MIDI File");
        
        // Grooves
        registerTooltip("groove_search", "Search Grooves by Name");
        registerTooltip("groove_tempo", "Filter by Tempo Range");
        registerTooltip("groove_favorite", "Toggle Favorite");
        registerTooltip("groove_play", "Preview Groove");
        
        // Routing
        registerTooltip("routing_bus", "Select Output Bus");
        registerTooltip("routing_link", "Link to DAW Output");
        registerTooltip("routing_send", "Send Level to FX Bus");
    }
    
private:
    TooltipManager() { initializeDefaultTooltips(); }
    ~TooltipManager() = default;
    TooltipManager(const TooltipManager&) = delete;
    TooltipManager& operator=(const TooltipManager&) = delete;
    
    std::map<juce::String, juce::String> tooltips;
};
