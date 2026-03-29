#pragma once
#include <juce_core/juce_core.h>

/** Canonical 15 mic bus indices: recording stems + Hi-Hat close. */
namespace MicBus
{
    static constexpr int count = 15;

    inline const char* rawName(int i) noexcept
    {
        static const char* names[count] = {
            "Kick In", "Kick Out", "Snare Top", "Snare Bottom",
            "Tom 1", "Tom 2", "Tom 3",
            "OH L", "OH R", "Room L", "Room R",
            "Room 2 R", "Room 2 L", "Scotch", "Hi-Hat"
        };
        return names[juce::jlimit(0, count - 1, i)];
    }

    inline juce::String getName(int i) { return juce::String(rawName(i)); }
}
