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

    // Palette accessors (dark = Pro Tools–style console; light = neutral studio)
    juce::Colour bg()        const { return c(0xFF2C2C2C, 0xFFF0F0F0); }
    juce::Colour panel()     const { return c(0xFF383838, 0xFFE8E8E8); }
    juce::Colour surface()   const { return c(0xFF404040, 0xFFFFFFFF); }
    juce::Colour surfaceHi() const { return c(0xFF4A4A4A, 0xFFF8F8F8); }
    juce::Colour border()    const { return c(0xFF1A1A1A, 0xFFC8C8C8); }
    juce::Colour borderHi()  const { return c(0xFF5C5C5C, 0xFFB0B0B0); }
    juce::Colour text()      const { return c(0xFFEAEAEA, 0xFF1A1A1A); }
    juce::Colour muted()     const { return c(0xFF9A9A9A, 0xFF666666); }
    juce::Colour dim()       const { return c(0xFF5C5C5C, 0xFF999999); }
    juce::Colour accent()    const { return juce::Colour(0xFFD4A74A); }
    juce::Colour accentDim() const { return juce::Colour(0xFF9A7A35); }

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
