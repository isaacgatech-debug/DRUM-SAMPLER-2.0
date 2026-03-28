#include "PluginBrowser.h"

PluginBrowser::PluginBrowser()
{
    searchBox.setTextToShowWhenEmpty("Search plugins...", juce::Colours::grey);
    searchBox.onTextChange = [this] { refreshList(); };
    addAndMakeVisible(searchBox);

    scanButton.onClick = [this]
    {
        if (pluginManager != nullptr)
            pluginManager->scanForPlugins();
        refreshList();
    };
    addAndMakeVisible(scanButton);

    pluginList.setModel(this);
    addAndMakeVisible(pluginList);
}

void PluginBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF1A1A1F));
}

void PluginBrowser::resized()
{
    auto area = getLocalBounds().reduced(8);
    auto top = area.removeFromTop(28);
    searchBox.setBounds(top.removeFromLeft(220));
    top.removeFromLeft(8);
    scanButton.setBounds(top.removeFromLeft(80));
    area.removeFromTop(6);
    pluginList.setBounds(area);
}

void PluginBrowser::setPluginManager(PluginManager* manager)
{
    pluginManager = manager;
    refreshList();
}

void PluginBrowser::refreshList()
{
    visiblePlugins.clear();
    if (pluginManager == nullptr)
    {
        pluginList.updateContent();
        pluginList.repaint();
        return;
    }

    const auto query = searchBox.getText().trim();
    visiblePlugins = query.isEmpty()
        ? pluginManager->getAvailablePlugins()
        : pluginManager->searchPlugins(query);
    pluginList.updateContent();
    pluginList.repaint();
}

int PluginBrowser::getNumRows()
{
    return static_cast<int>(visiblePlugins.size());
}

void PluginBrowser::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= static_cast<int>(visiblePlugins.size()))
        return;

    const auto& plugin = visiblePlugins[static_cast<size_t>(rowNumber)];
    g.fillAll(rowIsSelected ? juce::Colour(0xFF235A7A) : juce::Colours::transparentBlack);
    g.setColour(juce::Colours::white);
    g.drawText(plugin.name + " - " + plugin.manufacturerName,
               6, 0, width - 12, height, juce::Justification::centredLeft, true);
}

void PluginBrowser::listBoxItemDoubleClicked(int row, const juce::MouseEvent&)
{
    if (row < 0 || row >= static_cast<int>(visiblePlugins.size()))
        return;
    if (onPluginSelected)
        onPluginSelected(visiblePlugins[static_cast<size_t>(row)]);
}
