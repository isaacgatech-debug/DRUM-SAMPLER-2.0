#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginColors.h"

class SettingsView : public juce::Component
{
public:
    SettingsView();

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setCurrentKitName(const juce::String& kitName);
    void setImportStatus(const juce::String& text);
    void setDrummerProfileIndex(int index);
    void setPlayingStyleIndex(int index);

    std::function<void()> onImportFolder;
    std::function<void()> onCreateKit;
    std::function<void(int)> onDrummerProfileChanged;
    std::function<void(int)> onPlayingStyleChanged;

private:
    juce::Label titleLabel;
    juce::Label kitNameLabel;
    juce::Label importStatusLabel;
    juce::Label drummerLabel;
    juce::Label styleLabel;

    juce::TextButton importBtn { "Import Drum Pack Folder..." };
    juce::TextButton createKitBtn { "Create New Kit Folder..." };
    juce::ComboBox drummerProfileBox;
    juce::ComboBox playingStyleBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsView)
};
