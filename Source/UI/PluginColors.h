#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * PluginColors — Drum Tech “console” palette (Pro Tools–inspired).
 * Warm neutrals, amber highlight, muted track colours — not neon / sci‑fi.
 */
namespace PluginColors
{
    // Backgrounds (raised / recessed gray, similar to classic DAW mix windows)
    inline constexpr juce::uint32 pluginBg        = 0xFF2C2C2C;
    inline constexpr juce::uint32 pluginPanel      = 0xFF383838;
    inline constexpr juce::uint32 pluginSurface    = 0xFF404040;
    inline constexpr juce::uint32 pluginSurfaceHi  = 0xFF4A4A4A;

    // Borders (dark inset + soft highlight)
    inline constexpr juce::uint32 pluginBorder     = 0xFF1A1A1A;
    inline constexpr juce::uint32 pluginBorderHi   = 0xFF5C5C5C;

    // Accent — amber / gold (selection, value, transport emphasis)
    inline constexpr juce::uint32 accent           = 0xFFD4A74A;
    inline constexpr juce::uint32 accentDim        = 0xFF9A7A35;

    // Text
    inline constexpr juce::uint32 textPrimary      = 0xFFEAEAEA;
    inline constexpr juce::uint32 textMuted        = 0xFF9A9A9A;
    inline constexpr juce::uint32 textDim          = 0xFF5C5C5C;

    // VU Meter segments
    inline constexpr juce::uint32 meterGreen       = 0xFF22C55E;
    inline constexpr juce::uint32 meterYellow      = 0xFFEAB308;
    inline constexpr juce::uint32 meterRed         = 0xFFEF4444;

    // Solo / Mute button active states
    inline constexpr juce::uint32 soloActive       = 0xFFEAB308;   // yellow
    inline constexpr juce::uint32 muteActive       = 0xFFEF4444;   // red

    // Groove regions — muted, ink-like (readable on dark gray)
    inline constexpr juce::uint32 grooveVerse      = 0xFF6B8A9E;
    inline constexpr juce::uint32 grooveFill       = 0xFFB87A5C;
    inline constexpr juce::uint32 grooveChorus     = 0xFF8B7E9E;
    inline constexpr juce::uint32 groovePreChorus  = 0xFF6E9B7A;
    inline constexpr juce::uint32 grooveBridge     = 0xFF9B7A8A;

    // Channel IDs — desaturated “track colour” pastels (console-style)
    inline const juce::Colour channelColors[12] = {
        juce::Colour(0xFFB0706A), juce::Colour(0xFFB8885E),
        juce::Colour(0xFFC4A86A), juce::Colour(0xFF9BA86E),
        juce::Colour(0xFF7AA882), juce::Colour(0xFF6A9A9A),
        juce::Colour(0xFF6E8AB0), juce::Colour(0xFF7A82B0),
        juce::Colour(0xFF8A86A8), juce::Colour(0xFF9888A0),
        juce::Colour(0xFFA08898), juce::Colour(0xFFA09090),
    };
    inline const juce::Colour masterColor = juce::Colour(0xFFC9A24A);

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

/** Layout tokens shared across instrument panel and look-and-feel. */
namespace UIDesign
{
    inline constexpr int instrumentRowH           = 32;
    inline constexpr int instrumentSliderHeight   = 36;
    inline constexpr int linearSliderThumbRadius  = 15;
    inline constexpr float linearSliderTrackMax   = 11.0f;
}

/**
 * Monospace font helper for all numeric readouts.
 */
namespace PluginFonts
{
    inline juce::Font mono(float size = 13.5f)
    {
        return juce::Font(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain);
    }

    inline juce::Font label(float size = 12.5f)
    {
        return juce::Font(juce::FontOptions(size, juce::Font::bold));
    }
} // namespace PluginFonts
