#include "PluginEditor.h"

DrumSampler2Editor::DrumSampler2Editor(DrumSampler2Processor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(1400, 900);
    setResizable(true, true);
    setResizeLimits(1200, 800, 4000, 2000);

    // Load logo from resources
    auto resourcesFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
        .getParentDirectory().getChildFile("Resources");
    auto logoFile = resourcesFolder.getChildFile("LOGO/gridlock-logo.png");
    if (logoFile.existsAsFile())
        logoImage = juce::ImageFileFormat::loadFrom(logoFile);

    // Tab buttons — custom painted, no background color override here
    auto styleTab = [this](juce::TextButton& btn, int tabIdx)
    {
        btn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginPanel));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textMuted));
        btn.setColour(juce::TextButton::textColourOnId,  juce::Colour(PluginColors::accent));
        btn.onClick = [this, tabIdx] { switchTab(tabIdx); };
        addAndMakeVisible(btn);
    };

    styleTab(tabKit,     0);
    styleTab(tabMixer,   1);
    styleTab(tabTrigger, 2);
    styleTab(tabGrooves, 3);

    // Kit selector dropdown
    kitSelector.addItem("Default Kit", 1);
    kitSelector.addItem("Rock Kit",    2);
    kitSelector.addItem("Jazz Kit",    3);
    kitSelector.setSelectedId(1, juce::dontSendNotification);
    kitSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(PluginColors::pluginSurface));
    kitSelector.setColour(juce::ComboBox::textColourId,       juce::Colour(PluginColors::textPrimary));
    kitSelector.setColour(juce::ComboBox::outlineColourId,    juce::Colour(PluginColors::pluginBorder));
    kitSelector.setColour(juce::ComboBox::arrowColourId,      juce::Colour(PluginColors::accent));
    addAndMakeVisible(kitSelector);

    // Content views
    kitView.setProcessor(&processor);
    mixerView.setProcessor(&processor);
    addAndMakeVisible(kitView);
    addAndMakeVisible(grooveBrowser);
    addAndMakeVisible(mixerView);
    addAndMakeVisible(triggerUI);

    // MIDI roll always visible
    addAndMakeVisible(grooveTimeline);
    addAndMakeVisible(transportBar);

    ErrorLogger::getInstance().addListener(this);
    addKeyListener(this);
    LOG_INFO("Drum Tech initialized — new UI");

    switchTab(0);
    startTimer(33);
    resized();
}

DrumSampler2Editor::~DrumSampler2Editor()
{
    ErrorLogger::getInstance().removeListener(this);
    removeKeyListener(this);
}

void DrumSampler2Editor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(PluginColors::pluginBg));

    // Top navigation bar
    juce::Rectangle<int> topNav(0, 0, getWidth(), topNavH);
    g.setColour(juce::Colour(PluginColors::pluginPanel));
    g.fillRect(topNav);

    // Bottom border on topNav
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawHorizontalLine(topNavH - 1, 0.0f, static_cast<float>(getWidth()));

    // Logo (20x20) in top-left
    if (logoImage.isValid())
    {
        g.drawImage(logoImage, juce::Rectangle<float>(10.0f, 12.0f, 20.0f, 20.0f),
                    juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }

    // Branding: "GRIDLOCK" (white, bold) + " | " + "DRUM TECH" (cyan)
    float textX = 38.0f;
    float textY = static_cast<float>(topNavH / 2 - 10);

    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("GRIDLOCK", static_cast<int>(textX), static_cast<int>(textY), 100, 20,
               juce::Justification::centredLeft, false);

    g.setFont(PluginFonts::mono(12.0f));
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawText("|", static_cast<int>(textX + 100), static_cast<int>(textY), 16, 20,
               juce::Justification::centred, false);

    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.setColour(juce::Colour(PluginColors::accent));
    g.drawText("DRUM TECH", static_cast<int>(textX + 116), static_cast<int>(textY), 120, 20,
               juce::Justification::centredLeft, false);

    // Instrument bar
    juce::Rectangle<int> instrBar(0, topNavH, getWidth(), instrBarH);
    g.setColour(juce::Colour(PluginColors::pluginPanel).darker(0.1f));
    g.fillRect(instrBar);
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawHorizontalLine(topNavH + instrBarH - 1, 0.0f, static_cast<float>(getWidth()));

    // Paint active tab underline
    auto tabAreaBounds = juce::Rectangle<int>(260, 0, getWidth() - 380, topNavH);
    paintTabBar(g, tabAreaBounds);
}

void DrumSampler2Editor::paintTabBar(juce::Graphics& g, juce::Rectangle<int> area)
{
    juce::TextButton* tabs[] = { &tabKit, &tabMixer, &tabTrigger, &tabGrooves };
    for (int i = 0; i < 4; ++i)
    {
        if (i == activeTab)
        {
            auto tabBounds = tabs[i]->getBounds();
            // Cyan underline
            g.setColour(juce::Colour(PluginColors::accent));
            g.fillRect(tabBounds.getX(), tabBounds.getBottom() - 2, tabBounds.getWidth(), 2);
            // Slight cyan tint
            g.setColour(juce::Colour(PluginColors::accent).withAlpha(0.08f));
            g.fillRect(tabBounds);
        }
    }
    juce::ignoreUnused(area);
}

void DrumSampler2Editor::resized()
{
    auto area = getLocalBounds();

    // Top nav
    auto topNavArea = area.removeFromTop(topNavH);

    // Tab buttons in top nav (right of branding)
    int tabX = 260;
    int tabW = 90;
    int tabH = topNavH;
    tabKit.setBounds    (tabX,             0, tabW, tabH);
    tabMixer.setBounds  (tabX + tabW,      0, tabW, tabH);
    tabTrigger.setBounds(tabX + tabW * 2,  0, tabW, tabH);
    tabGrooves.setBounds(tabX + tabW * 3,  0, tabW, tabH);

    // Kit selector in top nav (far right)
    kitSelector.setBounds(getWidth() - 150, (topNavH - 26) / 2, 140, 26);

    // Instrument bar
    area.removeFromTop(instrBarH);

    // Bottom: transport (44px) + MIDI roll (134px) = 178px
    auto transportArea = area.removeFromBottom(transportBarH);
    auto rollArea      = area.removeFromBottom(midiRollH);

    transportBar.setBounds(transportArea);
    grooveTimeline.setBounds(rollArea);

    // Content area for tab views
    kitView.setBounds     (area);
    grooveBrowser.setBounds(area);
    mixerView.setBounds   (area);
    triggerUI.setBounds   (area);

    // Show/hide based on active tab (and update tab button text colour)
    kitView.setVisible     (activeTab == 0 && !debugConsoleVisible);
    grooveBrowser.setVisible(activeTab == 3 && !debugConsoleVisible);
    mixerView.setVisible   (activeTab == 1 && !debugConsoleVisible);
    triggerUI.setVisible   (activeTab == 2 && !debugConsoleVisible);

    if (debugConsole)
    {
        debugConsole->setVisible(debugConsoleVisible);
        if (debugConsoleVisible)
            debugConsole->setBounds(area);
    }

    repaint(); // repaint for tab underlines
    juce::ignoreUnused(topNavArea);
}

void DrumSampler2Editor::switchTab(int tab)
{
    activeTab = tab;

    // Update tab text colour
    juce::TextButton* tabs[] = { &tabKit, &tabMixer, &tabTrigger, &tabGrooves };
    for (int i = 0; i < 4; ++i)
    {
        bool active = (i == activeTab);
        tabs[i]->setColour(juce::TextButton::textColourOffId,
                           active ? juce::Colour(PluginColors::accent)
                                  : juce::Colour(PluginColors::textMuted));
    }

    resized();
}

bool DrumSampler2Editor::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    // Ctrl+Shift+D — toggle debug console (replaces BUGS button)
    if (key == juce::KeyPress('d', juce::ModifierKeys::ctrlModifier
                                 | juce::ModifierKeys::shiftModifier, 0))
    {
        toggleDebugConsole();
        return true;
    }
    return false;
}

void DrumSampler2Editor::toggleDebugConsole()
{
    debugConsoleVisible = !debugConsoleVisible;

    if (debugConsoleVisible)
    {
        if (!debugConsole)
        {
            debugConsole = std::make_unique<DebugConsole>();
            addAndMakeVisible(*debugConsole);
        }
        debugConsole->setVisible(true);
        debugConsole->toFront(true);
        LOG_INFO("Debug console opened");
    }
    else if (debugConsole)
    {
        debugConsole->setVisible(false);
    }

    resized();
    repaint();
}

void DrumSampler2Editor::updateStatus()
{
    int sampleCount = processor.getNumLoadedSamples();
    if (sampleCount > 0)
        LOG_INFO("Loaded " + juce::String(sampleCount) + " samples");
    else
        LOG_WARNING("No samples loaded");
}

void DrumSampler2Editor::logUpdated()
{
    // Log updated (debug console, no visible bugs button)
}
