#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ErrorLogger.h"

class DebugConsole : public juce::Component,
                     public ErrorLogger::Listener,
                     private juce::Timer
{
public:
    DebugConsole()
    {
        setSize(800, 600);
        
        addAndMakeVisible(logDisplay);
        logDisplay.setMultiLine(true);
        logDisplay.setReadOnly(true);
        logDisplay.setScrollbarsShown(true);
        logDisplay.setCaretVisible(false);
        logDisplay.setPopupMenuEnabled(true);
        logDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF1A1A1A));
        logDisplay.setColour(juce::TextEditor::textColourId, juce::Colours::lightgrey);
        
        addAndMakeVisible(clearButton);
        clearButton.setButtonText("Clear");
        clearButton.onClick = [this] { 
            juce::MessageManager::callAsync([this]() {
                ErrorLogger::getInstance().clearLogs();
                updateDisplay();
            });
        };
        
        addAndMakeVisible(exportButton);
        exportButton.setButtonText("Export Log");
        exportButton.onClick = [this] { exportLog(); };
        
        addAndMakeVisible(filterCombo);
        filterCombo.addItem("All", 1);
        filterCombo.addItem("Info", 2);
        filterCombo.addItem("Warnings", 3);
        filterCombo.addItem("Errors", 4);
        filterCombo.addItem("Critical", 5);
        filterCombo.setSelectedId(1);
        filterCombo.onChange = [this] { updateDisplay(); };
        
        addAndMakeVisible(autoScrollToggle);
        autoScrollToggle.setButtonText("Auto-scroll");
        autoScrollToggle.setToggleState(true, juce::dontSendNotification);
        
        addAndMakeVisible(statsLabel);
        statsLabel.setJustificationType(juce::Justification::centredLeft);
        
        juce::MessageManager::callAsync([this]() {
            ErrorLogger::getInstance().addListener(this);
            updateDisplay();
        });
        
        startTimer(1000);
    }
    
    ~DebugConsole() override
    {
        ErrorLogger::getInstance().removeListener(this);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xFF2A2A2A));
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(10);
        
        auto topBar = bounds.removeFromTop(30);
        clearButton.setBounds(topBar.removeFromLeft(80).reduced(2));
        exportButton.setBounds(topBar.removeFromLeft(100).reduced(2));
        topBar.removeFromLeft(10);
        filterCombo.setBounds(topBar.removeFromLeft(120).reduced(2));
        topBar.removeFromLeft(10);
        autoScrollToggle.setBounds(topBar.removeFromLeft(100).reduced(2));
        
        bounds.removeFromTop(5);
        
        auto bottomBar = bounds.removeFromBottom(25);
        statsLabel.setBounds(bottomBar);
        
        bounds.removeFromBottom(5);
        logDisplay.setBounds(bounds);
    }
    
    void logUpdated() override
    {
        updateDisplay();
    }
    
private:
    void timerCallback() override
    {
        updateStats();
    }
    
    void updateDisplay()
    {
        if (!juce::MessageManager::getInstance()->isThisTheMessageThread())
        {
            juce::MessageManager::callAsync([this]() { updateDisplay(); });
            return;
        }
        
        auto entries = ErrorLogger::getInstance().getLogEntries();
        
        juce::String text;
        int selectedFilter = filterCombo.getSelectedId();
        
        for (const auto& entry : entries)
        {
            bool include = false;
            
            switch (selectedFilter)
            {
                case 1: include = true; break;
                case 2: include = (entry.level == LogLevel::Info); break;
                case 3: include = (entry.level == LogLevel::Warning); break;
                case 4: include = (entry.level == LogLevel::Error); break;
                case 5: include = (entry.level == LogLevel::Critical); break;
            }
            
            if (include)
            {
                text << "[" << entry.timestamp << "] ";
                text << "[" << entry.getLevelString() << "] ";
                if (entry.source.isNotEmpty())
                    text << entry.source << ": ";
                text << entry.message << "\n";
            }
        }
        
        logDisplay.setText(text, false);
        
        if (autoScrollToggle.getToggleState())
        {
            logDisplay.moveCaretToEnd();
        }
        
        updateStats();
    }
    
    void updateStats()
    {
        auto entries = ErrorLogger::getInstance().getLogEntries();
        int errorCount = ErrorLogger::getInstance().getErrorCount();
        
        juce::String stats;
        stats << "Total Logs: " << entries.size() << " | ";
        stats << "Errors: " << errorCount << " | ";
        stats << "System Memory: " << (juce::SystemStats::getMemorySizeInMegabytes()) << " MB";
        
        statsLabel.setText(stats, juce::dontSendNotification);
    }
    
    void exportLog()
    {
        auto chooser = std::make_shared<juce::FileChooser>("Export Error Log", juce::File(), "*.txt");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [chooser](const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file != juce::File())
                {
                    ErrorLogger::getInstance().exportToFile(file);
                }
            });
    }
    
    juce::TextEditor logDisplay;
    juce::TextButton clearButton;
    juce::TextButton exportButton;
    juce::ComboBox filterCombo;
    juce::ToggleButton autoScrollToggle;
    juce::Label statsLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugConsole)
};
