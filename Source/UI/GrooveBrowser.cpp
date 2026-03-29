#include "GrooveBrowser.h"
#include "../Core/ErrorLogger.h"

GrooveBrowser::GrooveBrowser()
{
    listModel = std::make_unique<GrooveListModel>(*this);
    
    searchLabel.setText("Search:", juce::dontSendNotification);
    addAndMakeVisible(searchLabel);
    
    searchBox.setTextToShowWhenEmpty("Search grooves...", juce::Colours::grey);
    searchBox.setFont(PluginFonts::mono(13.0f));
    searchBox.onTextChange = [this] { filterGrooves(); };
    addAndMakeVisible(searchBox);
    
    categoryLabel.setText("Category:", juce::dontSendNotification);
    addAndMakeVisible(categoryLabel);
    
    categoryFilter.addItem("All Categories", 1);
    categoryFilter.setSelectedId(1);
    categoryFilter.onChange = [this] { filterGrooves(); };
    addAndMakeVisible(categoryFilter);
    
    tempoLabel.setText("Tempo Range:", juce::dontSendNotification);
    addAndMakeVisible(tempoLabel);
    
    tempoMinSlider.setRange(60, 200, 1);
    tempoMinSlider.setValue(60);
    tempoMinSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tempoMinSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 56, 24);
    tempoMinSlider.onValueChange = [this] { filterGrooves(); };
    addAndMakeVisible(tempoMinSlider);
    
    tempoMaxSlider.setRange(60, 200, 1);
    tempoMaxSlider.setValue(200);
    tempoMaxSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tempoMaxSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 56, 24);
    tempoMaxSlider.onValueChange = [this] { filterGrooves(); };
    addAndMakeVisible(tempoMaxSlider);
    
    favoritesOnly.setButtonText("Favorites Only");
    favoritesOnly.onClick = [this] { filterGrooves(); };
    addAndMakeVisible(favoritesOnly);
    
    grooveList.setModel(listModel.get());
    grooveList.setColour(juce::ListBox::backgroundColourId, listBgColour);
    grooveList.setRowHeight(38);
    addAndMakeVisible(grooveList);
    
    scanButton.onClick = [this] {
        if (grooveLibrary)
        {
            grooveLibrary->scanFolders();
            updateGrooveList();
            LOG_INFO("Groove library scanned");
        }
    };
    addAndMakeVisible(scanButton);
    
    previewButton.onClick = [this] {
        if (selectedGrooveIndex >= 0 && selectedGrooveIndex < static_cast<int>(filteredGrooves.size()))
            previewGroove(&filteredGrooves[static_cast<size_t>(selectedGrooveIndex)]);
    };
    addAndMakeVisible(previewButton);
    
    addToTimelineButton.onClick = [this] {
        if (selectedGrooveIndex >= 0 && selectedGrooveIndex < static_cast<int>(filteredGrooves.size()))
        {
            const auto& selectedGroove = filteredGrooves[static_cast<size_t>(selectedGrooveIndex)];
            if (onAddToTimeline)
                onAddToTimeline(selectedGroove);
            LOG_INFO("Add to timeline: " + selectedGroove.name);
        }
    };
    addAndMakeVisible(addToTimelineButton);
    
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statusLabel);
    
    startTimer(100);
}

void GrooveBrowser::paint(juce::Graphics& g)
{
    g.fillAll(bgColour);
    
    g.setColour(juce::Colours::white);
    g.setFont(PluginFonts::label(22.0f));
    g.drawText("Groove Browser", 14, 12, 280, 36, juce::Justification::centredLeft);
}

void GrooveBrowser::resized()
{
    auto bounds = getLocalBounds().reduced(14);
    bounds.removeFromTop(48);
    
    auto topControls = bounds.removeFromTop(96);
    
    auto searchRow = topControls.removeFromTop(30);
    searchLabel.setBounds(searchRow.removeFromLeft(72));
    searchBox.setBounds(searchRow.removeFromLeft(240));
    searchRow.removeFromLeft(24);
    categoryLabel.setBounds(searchRow.removeFromLeft(84));
    categoryFilter.setBounds(searchRow.removeFromLeft(180));
    searchRow.removeFromLeft(24);
    favoritesOnly.setBounds(searchRow.removeFromLeft(140));
    
    topControls.removeFromTop(8);
    
    auto tempoRow = topControls.removeFromTop(30);
    tempoLabel.setBounds(tempoRow.removeFromLeft(120));
    tempoMinSlider.setBounds(tempoRow.removeFromLeft(180));
    tempoRow.removeFromLeft(14);
    tempoMaxSlider.setBounds(tempoRow.removeFromLeft(180));
    
    topControls.removeFromTop(8);
    
    auto buttonRow = topControls.removeFromTop(36);
    scanButton.setBounds(buttonRow.removeFromLeft(140));
    buttonRow.removeFromLeft(12);
    previewButton.setBounds(buttonRow.removeFromLeft(120));
    buttonRow.removeFromLeft(12);
    addToTimelineButton.setBounds(buttonRow.removeFromLeft(160));
    
    bounds.removeFromTop(12);
    
    auto bottomBar = bounds.removeFromBottom(30);
    statusLabel.setBounds(bottomBar);
    statusLabel.setFont(PluginFonts::mono(12.0f));
    
    bounds.removeFromBottom(6);
    grooveList.setBounds(bounds);
}

void GrooveBrowser::setGrooveLibrary(GrooveLibrary* library)
{
    grooveLibrary = library;
    updateGrooveList();
}

void GrooveBrowser::setMIDIPlayer(MIDIPlayer* player)
{
    midiPlayer = player;
}

void GrooveBrowser::timerCallback()
{
    if (midiPlayer && midiPlayer->isPlaying())
    {
        statusLabel.setText("Playing: " + juce::String(midiPlayer->getPosition(), 1) + " / " + 
                          juce::String(midiPlayer->getLength(), 1) + " beats", 
                          juce::dontSendNotification);
    }
    else
    {
        statusLabel.setText(juce::String(filteredGrooves.size()) + " grooves", 
                          juce::dontSendNotification);
    }
}

void GrooveBrowser::updateGrooveList()
{
    if (!grooveLibrary)
        return;
    
    auto categories = grooveLibrary->getAllGrooves();
    categoryFilter.clear();
    categoryFilter.addItem("All Categories", 1);
    
    std::set<juce::String> uniqueCategories;
    for (const auto& groove : categories)
    {
        for (const auto& tag : groove.genreTags)
            uniqueCategories.insert(tag);
    }
    
    int id = 2;
    for (const auto& cat : uniqueCategories)
    {
        categoryFilter.addItem(cat, id++);
    }
    
    filterGrooves();
}

void GrooveBrowser::filterGrooves()
{
    if (!grooveLibrary)
        return;
    
    filteredGrooves.clear();
    selectedGrooveIndex = -1;
    
    auto allGrooves = grooveLibrary->getAllGrooves();
    juce::String searchText = searchBox.getText().toLowerCase();
    int minTempo = static_cast<int>(tempoMinSlider.getValue());
    int maxTempo = static_cast<int>(tempoMaxSlider.getValue());
    bool favOnly = favoritesOnly.getToggleState();
    juce::String selectedCategory = categoryFilter.getText();
    
    for (const auto& groove : allGrooves)
    {
        bool matches = true;
        
        if (favOnly && !groove.isFavorite)
            matches = false;
        
        if (!searchText.isEmpty() && !groove.name.toLowerCase().contains(searchText))
            matches = false;
        
        if (groove.tempoBPM < minTempo || groove.tempoBPM > maxTempo)
            matches = false;
        
        if (selectedCategory != "All Categories")
        {
            bool hasCategory = false;
            for (const auto& tag : groove.genreTags)
            {
                if (tag == selectedCategory)
                {
                    hasCategory = true;
                    break;
                }
            }
            if (!hasCategory)
                matches = false;
        }
        
        if (matches)
            filteredGrooves.push_back(groove);
    }
    
    grooveList.updateContent();
    grooveList.repaint();
}

void GrooveBrowser::previewGroove(const GrooveMetadata* groove)
{
    if (!midiPlayer || !groove)
        return;
    
    midiPlayer->loadGroove(groove);
    midiPlayer->setLoop(true);
    midiPlayer->play();
    
    LOG_INFO("Previewing groove: " + groove->name);
}
