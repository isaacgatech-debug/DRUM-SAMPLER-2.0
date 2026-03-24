#include "PluginEditor.h"

DrumSampler2Editor::DrumSampler2Editor(DrumSampler2Processor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(650, 750);

    auto resourcesFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
        .getParentDirectory().getChildFile("Resources");
    auto logoFile = resourcesFolder.getChildFile("gridlock-logo.png");
    
    if (logoFile.existsAsFile())
    {
        logoImage = juce::ImageFileFormat::loadFrom(logoFile);
    }

    titleLabel.setText("DRUM SAMPLER 2", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, textCol);
    addAndMakeVisible(titleLabel);

    loadButton.setColour(juce::TextButton::buttonColourId, accent);
    loadButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    loadButton.onClick = [this] { loadSamplesClicked(); };
    addAndMakeVisible(loadButton);
    
    bugsButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    bugsButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    bugsButton.onClick = [this] { toggleDebugConsole(); };
    addAndMakeVisible(bugsButton);

    statusLabel.setColour(juce::Label::textColourId, textCol);
    addAndMakeVisible(statusLabel);
    
    ErrorLogger::getInstance().addListener(this);
    updateBugsButton();
    
    LOG_INFO("Drum Sampler 2.0 initialized");

    tabKit.setColour(juce::TextButton::buttonColourId, accent);
    tabKit.onClick = [this] { activeTab = 0; resized(); };
    addAndMakeVisible(tabKit);

    tabGrooves.setColour(juce::TextButton::buttonColourId, header);
    tabGrooves.onClick = [this] { activeTab = 1; resized(); };
    addAndMakeVisible(tabGrooves);

    tabMixer.setColour(juce::TextButton::buttonColourId, header);
    tabMixer.onClick = [this] { activeTab = 2; resized(); };
    addAndMakeVisible(tabMixer);

    tabTrigger.setColour(juce::TextButton::buttonColourId, header);
    tabTrigger.onClick = [this] { activeTab = 3; resized(); };
    addAndMakeVisible(tabTrigger);

    tabRouting.setColour(juce::TextButton::buttonColourId, header);
    tabRouting.onClick = [this] { activeTab = 4; resized(); };
    addAndMakeVisible(tabRouting);

    kitView.setProcessor(&processor);
    addAndMakeVisible(kitView);
    addAndMakeVisible(grooveBrowser);
    addAndMakeVisible(mixerView);
    addAndMakeVisible(triggerUI);
    addAndMakeVisible(routingView);

    updateStatus();
    
    // Start timer to poll for MIDI events (30 FPS)
    startTimer(33);
    
    // Ensure initial layout is applied and KIT tab is visible
    resized();
}

DrumSampler2Editor::~DrumSampler2Editor()
{
    ErrorLogger::getInstance().removeListener(this);
}

void DrumSampler2Editor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    
    g.setColour(header);
    g.fillRect(0, 0, getWidth(), 60);
    
    if (logoImage.isValid())
    {
        auto logoArea = juce::Rectangle<int>(10, 10, 150, 40);
        g.drawImage(logoImage, logoArea.toFloat(), 
                   juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }
    
    g.setColour(header);
    g.fillRect(0, 60, getWidth(), 40);
}

void DrumSampler2Editor::resized()
{
    auto area = getLocalBounds();
    
    auto headerArea = area.removeFromTop(60);
    headerArea.removeFromLeft(170);
    titleLabel.setBounds(headerArea.removeFromLeft(200).reduced(10));
    bugsButton.setBounds(headerArea.removeFromRight(80).reduced(10));
    loadButton.setBounds(headerArea.removeFromRight(150).reduced(10));
    statusLabel.setBounds(headerArea.reduced(10));
    
    auto tabArea = area.removeFromTop(40);
    int tabWidth = tabArea.getWidth() / 5;
    tabKit.setBounds(tabArea.removeFromLeft(tabWidth));
    tabGrooves.setBounds(tabArea.removeFromLeft(tabWidth));
    tabMixer.setBounds(tabArea.removeFromLeft(tabWidth));
    tabTrigger.setBounds(tabArea.removeFromLeft(tabWidth));
    tabRouting.setBounds(tabArea);
    
    tabKit.setColour(juce::TextButton::buttonColourId, activeTab == 0 ? accent : header);
    tabGrooves.setColour(juce::TextButton::buttonColourId, activeTab == 1 ? accent : header);
    tabMixer.setColour(juce::TextButton::buttonColourId, activeTab == 2 ? accent : header);
    tabTrigger.setColour(juce::TextButton::buttonColourId, activeTab == 3 ? accent : header);
    tabRouting.setColour(juce::TextButton::buttonColourId, activeTab == 4 ? accent : header);
    
    kitView.setBounds(area);
    grooveBrowser.setBounds(area);
    mixerView.setBounds(area);
    triggerUI.setBounds(area);
    routingView.setBounds(area);
    
    kitView.setVisible(activeTab == 0 && !debugConsoleVisible);
    grooveBrowser.setVisible(activeTab == 1 && !debugConsoleVisible);
    mixerView.setVisible(activeTab == 2 && !debugConsoleVisible);
    triggerUI.setVisible(activeTab == 3 && !debugConsoleVisible);
    routingView.setVisible(activeTab == 4 && !debugConsoleVisible);
    
    if (debugConsole && debugConsoleVisible)
    {
        debugConsole->setVisible(true);
        debugConsole->setBounds(area);
    }
    else if (debugConsole)
    {
        debugConsole->setVisible(false);
    }
}

void DrumSampler2Editor::loadSamplesClicked()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Select sample folder",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        ""
    );

    auto flags = juce::FileBrowserComponent::openMode | 
                 juce::FileBrowserComponent::canSelectDirectories;

    chooser->launchAsync(flags, [this, chooser](const juce::FileChooser& fc)
    {
        auto folder = fc.getResult();
        if (folder.exists())
        {
            processor.loadSamplesFromFolder(folder);
            updateStatus();
        }
    });
}

void DrumSampler2Editor::updateStatus()
{
    int sampleCount = processor.getNumLoadedSamples();
    statusLabel.setText("Samples loaded: " + juce::String(sampleCount), juce::dontSendNotification);
    
    if (sampleCount > 0)
    {
        LOG_INFO("Loaded " + juce::String(sampleCount) + " samples successfully");
    }
    else
    {
        LOG_WARNING("No samples loaded. Check folder contains .wav, .aif, or .aiff files");
    }
}

void DrumSampler2Editor::toggleDebugConsole()
{
    debugConsoleVisible = !debugConsoleVisible;
    
    DBG("Toggle debug console: " << (debugConsoleVisible ? "SHOW" : "HIDE"));
    
    if (debugConsoleVisible)
    {
        if (!debugConsole)
        {
            debugConsole = std::make_unique<DebugConsole>();
            addAndMakeVisible(*debugConsole);
            DBG("Created debug console");
        }
        debugConsole->setVisible(true);
        debugConsole->toFront(true);
        LOG_INFO("Debug console opened");
    }
    else if (debugConsole)
    {
        debugConsole->setVisible(false);
    }
    
    bugsButton.setButtonText(debugConsoleVisible ? "CLOSE" : "BUGS");
    resized();
    repaint();
}

void DrumSampler2Editor::updateBugsButton()
{
    int errorCount = ErrorLogger::getInstance().getErrorCount();
    
    if (errorCount > 0)
    {
        bugsButton.setButtonText("BUGS (" + juce::String(errorCount) + ")");
        bugsButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    }
    else
    {
        bugsButton.setButtonText(debugConsoleVisible ? "CLOSE" : "BUGS");
        bugsButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    }
}

void DrumSampler2Editor::logUpdated()
{
    updateBugsButton();
}
