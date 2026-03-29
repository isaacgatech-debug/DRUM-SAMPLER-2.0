#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginColors.h"
#include "../Grooves/GrooveLibrary.h"
#include "../Grooves/MIDIPlayer.h"

class GrooveBrowser : public juce::Component,
                      private juce::Timer
{
public:
    GrooveBrowser();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setGrooveLibrary(GrooveLibrary* library);
    void setMIDIPlayer(MIDIPlayer* player);

    std::function<void(const GrooveMetadata&)> onAddToTimeline;
    
private:
    void timerCallback() override;
    void updateGrooveList();
    void filterGrooves();
    void previewGroove(const GrooveMetadata* groove);
    
    GrooveLibrary* grooveLibrary = nullptr;
    MIDIPlayer* midiPlayer = nullptr;
    
    juce::TextEditor searchBox;
    juce::Label searchLabel;
    
    juce::ComboBox categoryFilter;
    juce::Label categoryLabel;
    
    juce::Slider tempoMinSlider;
    juce::Slider tempoMaxSlider;
    juce::Label tempoLabel;
    
    juce::ToggleButton favoritesOnly;
    
    juce::ListBox grooveList;
    juce::TextButton scanButton{"Scan Folders"};
    juce::TextButton previewButton{"Preview"};
    juce::TextButton addToTimelineButton{"Add to Timeline"};
    
    juce::Label statusLabel;
    
    std::vector<GrooveMetadata> filteredGrooves;
    int selectedGrooveIndex = -1;
    
    juce::Colour bgColour{0xFF2A2A2A};
    juce::Colour listBgColour{0xFF1A1A1A};
    juce::Colour accentColour{juce::Colour(PluginColors::accent)};
    
    class GrooveListModel : public juce::ListBoxModel
    {
    public:
        GrooveListModel(GrooveBrowser& owner) : browser(owner) {}
        
        int getNumRows() override
        {
            return static_cast<int>(browser.filteredGrooves.size());
        }
        
        void paintListBoxItem(int rowNumber, juce::Graphics& g,
                            int width, int height, bool rowIsSelected) override
        {
            if (rowNumber < 0 || rowNumber >= static_cast<int>(browser.filteredGrooves.size()))
                return;
            
            const auto& groove = browser.filteredGrooves[rowNumber];
            
            if (rowIsSelected)
                g.fillAll(browser.accentColour.withAlpha(0.3f));
            else if (rowNumber % 2 == 0)
                g.fillAll(juce::Colour(0xFF252525));
            
            g.setColour(juce::Colours::white);
            g.setFont(PluginFonts::label(14.0f));
            
            auto textArea = juce::Rectangle<int>(8, 0, width - 16, height);
            g.drawText(groove.name, textArea.removeFromLeft(width / 2), 
                      juce::Justification::centredLeft, true);
            
            g.setColour(juce::Colours::lightgrey);
            g.setFont(PluginFonts::mono(12.5f));
            g.drawText(juce::String(groove.tempoBPM) + " BPM", 
                      textArea.removeFromLeft(96), 
                      juce::Justification::centredLeft, true);
            
            if (groove.isFavorite)
            {
                g.setColour(juce::Colours::gold);
                g.fillEllipse(width - 25.0f, height / 2.0f - 5.0f, 10.0f, 10.0f);
            }
        }
        
        void listBoxItemClicked(int row, const juce::MouseEvent&) override
        {
            if (row >= 0 && row < static_cast<int>(browser.filteredGrooves.size()))
            {
                browser.selectedGrooveIndex = row;
            }
        }
        
        void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override
        {
            if (row >= 0 && row < static_cast<int>(browser.filteredGrooves.size()))
            {
                browser.previewGroove(&browser.filteredGrooves[row]);
            }
        }
        
    private:
        GrooveBrowser& browser;
    };
    
    std::unique_ptr<GrooveListModel> listModel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveBrowser)
};
