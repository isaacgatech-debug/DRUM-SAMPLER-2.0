#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Effects/PluginManager.h"

class PluginBrowser : public juce::Component, private juce::ListBoxModel
{
public:
    PluginBrowser();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setPluginManager(PluginManager* manager);
    void refreshList();

    std::function<void(const juce::PluginDescription&)> onPluginSelected;
    
private:
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;

    PluginManager* pluginManager = nullptr;
    std::vector<juce::PluginDescription> visiblePlugins;

    juce::TextEditor searchBox;
    juce::TextButton scanButton { "Scan" };
    juce::ListBox pluginList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginBrowser)
};
