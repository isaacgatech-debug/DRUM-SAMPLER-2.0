#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * PluginColors — Drum Tech design token system
 * All juce::Colour constants for the plugin UI.
 * Replace all orange (0xFFE8A020) usage with these tokens.
 */
namespace PluginColors
{
    // Backgrounds
    inline constexpr juce::uint32 pluginBg        = 0xFF0D0D0E;
    inline constexpr juce::uint32 pluginPanel      = 0xFF141416;
    inline constexpr juce::uint32 pluginSurface    = 0xFF1C1C1F;
    inline constexpr juce::uint32 pluginSurfaceHi  = 0xFF242428;

    // Borders
    inline constexpr juce::uint32 pluginBorder     = 0xFF2C2C32;
    inline constexpr juce::uint32 pluginBorderHi   = 0xFF3C3C44;

    // Accent — cyan replaces all orange
    inline constexpr juce::uint32 accent           = 0xFF00C8FF;
    inline constexpr juce::uint32 accentDim        = 0xFF0088CC;

    // Text
    inline constexpr juce::uint32 textPrimary      = 0xFFDDE1E7;
    inline constexpr juce::uint32 textMuted        = 0xFF6B7280;
    inline constexpr juce::uint32 textDim          = 0xFF3A3A44;

    // VU Meter segments
    inline constexpr juce::uint32 meterGreen       = 0xFF22C55E;
    inline constexpr juce::uint32 meterYellow      = 0xFFEAB308;
    inline constexpr juce::uint32 meterRed         = 0xFFEF4444;

    // Solo / Mute button active states
    inline constexpr juce::uint32 soloActive       = 0xFFEAB308;   // yellow
    inline constexpr juce::uint32 muteActive       = 0xFFEF4444;   // red

    // MIDI Groove block colors (by type)
    inline constexpr juce::uint32 grooveVerse      = 0xFF0EA5E9;
    inline constexpr juce::uint32 grooveFill       = 0xFFF97316;
    inline constexpr juce::uint32 grooveChorus     = 0xFFA855F7;
    inline constexpr juce::uint32 groovePreChorus  = 0xFF22C55E;
    inline constexpr juce::uint32 grooveBridge     = 0xFFEC4899;

    // Channel strip colors (index 0-11 + master)
    inline const juce::Colour channelColors[12] = {
        juce::Colour(0xFFEF4444),  // 0  Kick In
        juce::Colour(0xFFF97316),  // 1  Kick Out
        juce::Colour(0xFFEAB308),  // 2  Snare Top
        juce::Colour(0xFF84CC16),  // 3  Snare Bot
        juce::Colour(0xFF22C55E),  // 4  Hi-Hat
        juce::Colour(0xFF06B6D4),  // 5  Tom 1
        juce::Colour(0xFF3B82F6),  // 6  Tom 2
        juce::Colour(0xFF6366F1),  // 7  Tom 3
        juce::Colour(0xFF8B5CF6),  // 8  OVH L
        juce::Colour(0xFFA855F7),  // 9  OVH R
        juce::Colour(0xFFEC4899),  // 10 Room L
        juce::Colour(0xFFEC4899),  // 11 Room R
    };
    inline const juce::Colour masterColor = juce::Colour(0xFF00C8FF);

    // Groove type -> color
    inline juce::Colour grooveColor(int type)
    {
        switch (type)
        {
            case 0:  return juce::Colour(grooveVerse);
            case 1:  return juce::Colour(grooveFill);
            case 2:  return juce::Colour(grooveChorus);
            case 3:  return juce::Colour(groovePreChorus);
            case 4:  return juce::Colour(grooveBridge);
            default: return juce::Colour(grooveVerse);
        }
    }

    // Groove type -> name
    inline juce::String grooveTypeName(int type)
    {
        switch (type)
        {
            case 0:  return "Verse";
            case 1:  return "Fill";
            case 2:  return "Chorus";
            case 3:  return "Pre-Chorus";
            case 4:  return "Bridge";
            default: return "Verse";
        }
    }

    // Convenience constructors
    inline juce::Colour bg()        { return juce::Colour(pluginBg); }
    inline juce::Colour panel()     { return juce::Colour(pluginPanel); }
    inline juce::Colour surface()   { return juce::Colour(pluginSurface); }
    inline juce::Colour surfaceHi() { return juce::Colour(pluginSurfaceHi); }
    inline juce::Colour border()    { return juce::Colour(pluginBorder); }
    inline juce::Colour borderHi()  { return juce::Colour(pluginBorderHi); }
    inline juce::Colour cyan()      { return juce::Colour(accent); }
    inline juce::Colour cyanDim()   { return juce::Colour(accentDim); }
    inline juce::Colour text()      { return juce::Colour(textPrimary); }
    inline juce::Colour muted()     { return juce::Colour(textMuted); }

} // namespace PluginColors

/**
 * Monospace font helper for all numeric readouts.
 */
namespace PluginFonts
{
    inline juce::Font mono(float size = 11.0f)
    {
        return juce::Font(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain);
    }

    inline juce::Font label(float size = 10.0f)
    {
        return juce::Font(juce::FontOptions(size, juce::Font::bold));
    }
} // namespace PluginFonts
