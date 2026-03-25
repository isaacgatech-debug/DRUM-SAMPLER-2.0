#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Global light/dark theme singleton.
 * Components call ThemeManager::get() to read current palette.
 * Set onThemeChanged callback to receive repaint notifications.
 */
class ThemeManager
{
public:
    static ThemeManager& get()
    {
        static ThemeManager inst;
        return inst;
    }

    bool isDark() const { return darkMode; }

    void toggle()
    {
        darkMode = !darkMode;
        if (onThemeChanged) onThemeChanged();
    }

    void setDark(bool d)
    {
        darkMode = d;
        if (onThemeChanged) onThemeChanged();
    }

    // Palette accessors
    juce::Colour bg()        const { return c(0xFF0D0D0E, 0xFFF2F2F6); }
    juce::Colour panel()     const { return c(0xFF141416, 0xFFE4E4EA); }
    juce::Colour surface()   const { return c(0xFF1C1C1F, 0xFFFFFFFF); }
    juce::Colour surfaceHi() const { return c(0xFF242428, 0xFFF6F6FA); }
    juce::Colour border()    const { return c(0xFF2C2C32, 0xFFCECED8); }
    juce::Colour borderHi()  const { return c(0xFF3C3C44, 0xFFB0B0BC); }
    juce::Colour text()      const { return c(0xFFDDE1E7, 0xFF1A1A2E); }
    juce::Colour muted()     const { return c(0xFF6B7280, 0xFF8888A0); }
    juce::Colour dim()       const { return c(0xFF3A3A44, 0xFFCCCCDC); }
    juce::Colour accent()    const { return juce::Colour(0xFF00C8FF); }
    juce::Colour accentDim() const { return juce::Colour(0xFF0088CC); }

    std::function<void()> onThemeChanged;

private:
    ThemeManager() = default;

    juce::Colour c(juce::uint32 dark, juce::uint32 light) const
    {
        return juce::Colour(darkMode ? dark : light);
    }

    bool darkMode = true;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemeManager)
};
