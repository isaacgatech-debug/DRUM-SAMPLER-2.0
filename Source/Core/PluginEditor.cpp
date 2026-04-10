#include "PluginEditor.h"
#include "BinaryData.h"
#include "../UI/ThemeManager.h"

DrumTechEditor::DrumTechEditor(DrumTechProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&appLookAndFeel);

    setSize(1480, 860);
    setResizable(true, true);
    setResizeLimits(1180, 740, 2800, 1700);

    // Load logo from binary data (preferred) or filesystem
#if defined(BinaryData_Gridlock_Logo___White_png)
    logoImage = juce::ImageFileFormat::loadFrom(
        BinaryData::Gridlock_Logo___White_png,
        BinaryData::Gridlock_Logo___White_pngSize);
#else
    // Filesystem fallback — works in development builds
    auto searchPaths = juce::Array<juce::File>{
        juce::File::getCurrentWorkingDirectory().getChildFile("Resources/LOGO/Gridlock Logo - White.png"),
        juce::File(__FILE__).getParentDirectory().getParentDirectory()
            .getChildFile("Resources/LOGO/Gridlock Logo - White.png")
    };
    for (auto& f : searchPaths)
        if (f.existsAsFile()) { logoImage = juce::ImageFileFormat::loadFrom(f); break; }
#endif

    // Load icon images from binary data
    homeIconImg = juce::ImageFileFormat::loadFrom(BinaryData::home_png, BinaryData::home_pngSize);
    kitBuilderIconImg = juce::ImageFileFormat::loadFrom(BinaryData::kitbuilder_png, BinaryData::kitbuilder_pngSize);
    mixerIconImg = juce::ImageFileFormat::loadFrom(BinaryData::mixer_png, BinaryData::mixer_pngSize);
    triggerIconImg = juce::ImageFileFormat::loadFrom(BinaryData::trigger_png, BinaryData::trigger_pngSize);
    settingsIconImg = juce::ImageFileFormat::loadFrom(BinaryData::settings_png, BinaryData::settings_pngSize);

    // Theme toggle button
    auto& tm = ThemeManager::get();
    tm.onThemeChanged = [this]
    {
        applyTheme();
        repaint();
    };

    themeModeBtn.setButtonText(tm.isDark() ? "LIGHT" : "DARK");
    themeModeBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
    themeModeBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::accent));
    themeModeBtn.onClick = [this]
    {
        ThemeManager::get().toggle();
        themeModeBtn.setButtonText(ThemeManager::get().isDark() ? "LIGHT" : "DARK");
        repaint();
    };
    addAndMakeVisible(themeModeBtn);

    // Tab buttons - icon-only, transparent backgrounds
    auto styleTab = [](juce::TextButton& btn, int tabIdx, std::function<void(int)> switchFn)
    {
        btn.setColour(juce::TextButton::buttonColourId,  juce::Colours::transparentBlack);
        btn.setColour(juce::TextButton::textColourOffId, juce::Colours::transparentBlack);
        btn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
        btn.onClick = [tabIdx, switchFn] { switchFn(tabIdx); };
    };

    auto switchFn = [this](int t) { switchTab(t); };
    styleTab(tabHome,       0, switchFn); addAndMakeVisible(tabHome);
    styleTab(tabKitBuilder, 1, switchFn); addAndMakeVisible(tabKitBuilder);
    styleTab(tabMixer,      2, switchFn); addAndMakeVisible(tabMixer);
    styleTab(tabTrigger,    3, switchFn); addAndMakeVisible(tabTrigger);
    styleTab(tabSettings,   4, switchFn); addAndMakeVisible(tabSettings);

    // Kit selector (shows popup menu instead of combo)
    kitSelectorBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
    kitSelectorBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textPrimary));
    kitSelectorBtn.onClick = [this] { showKitPopupMenu(); };
    addAndMakeVisible(kitSelectorBtn);

    kitLockBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(PluginColors::pluginSurface));
    kitLockBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textMuted));
    kitLockBtn.onClick = [this]
    {
        kitLocked = !kitLocked;
        kitLockBtn.setButtonText(kitLocked ? "LOCK: ON" : "LOCK: OFF");
        kitLockBtn.setColour(juce::TextButton::textColourOffId,
                             kitLocked ? juce::Colour(PluginColors::accent)
                                       : juce::Colour(PluginColors::textMuted));
        settingsView.setImportStatus(kitLocked ? "kit locked" : "kit unlocked");
    };
    addAndMakeVisible(kitLockBtn);

    // Content views
    kitView.setProcessor(&processor);
    kitView.setKitBuilderMode(false);
    kitView.onMixerPressed = [this]
    {
        mixerSlideTarget = (mixerSlideTarget > 0.5f) ? 0.0f : 1.0f;
    };
    kitView.onHomePressed = [this]
    {
        mixerSlideTarget = 0.0f;
    };
    mixerView.setProcessor(&processor);
    mixerView.setAlpha(0.86f);
    grooveBrowser.setGrooveLibrary(&processor.getGrooveLibrary());
    grooveBrowser.setMIDIPlayer(&processor.getMIDIPlayer());
    grooveBrowser.onAddToTimeline = [this](const GrooveMetadata& groove)
    {
        GrooveTimeline::GrooveBlock block;
        block.name = groove.name;
        block.durationBars = juce::jmax(1, groove.lengthInBeats / 4);
        block.type = 0;
        block.startBar = grooveTimeline.getBlocks().empty()
            ? 1
            : grooveTimeline.getBlocks().back().startBar + grooveTimeline.getBlocks().back().durationBars;
        grooveTimeline.addBlock(block);
    };
    addAndMakeVisible(kitView);
    addAndMakeVisible(mixerDismissLayer);
    addAndMakeVisible(mixerView);
    addAndMakeVisible(settingsView);
    addAndMakeVisible(triggerView);

    mixerDismissLayer.onDismiss = [this]
    {
        mixerSlideTarget = 0.0f;
    };

    settingsView.onImportFolder = [this] { showKitPopupMenu(); };
    settingsView.onCreateKit = [this] { showKitPopupMenu(); };
    settingsView.onDrummerProfileChanged = [this](int idx)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(
                processor.getAPVTS().getParameter("samplerDrummerProfile")))
        {
            const int n = juce::jmax(1, p->choices.size() - 1);
            p->setValueNotifyingHost(static_cast<float>(juce::jlimit(0, n, idx)) / static_cast<float>(n));
        }
    };
    settingsView.onPlayingStyleChanged = [this](int idx)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(
                processor.getAPVTS().getParameter("samplerPlayingStyle")))
        {
            const int n = juce::jmax(1, p->choices.size() - 1);
            p->setValueNotifyingHost(static_cast<float>(juce::jlimit(0, n, idx)) / static_cast<float>(n));
        }
    };

    // MIDI roll + transport (always visible)
    addAndMakeVisible(grooveTimeline);
    addAndMakeVisible(transportBar);
    transportBar.onPlay = [this](bool shouldPlay)
    {
        if (shouldPlay)
        {
            auto seq = grooveTimeline.buildSequenceForGrid();
            if (seq.getNumEvents() > 0)
                processor.getMIDIPlayer().loadSequence(seq, processor.getMIDIPlayer().getTempo());
        }
        processor.setTransportPlaying(shouldPlay);
    };
    transportBar.onStop = [this] { processor.setTransportPlaying(false); };
    transportBar.onLoop = [this](bool shouldLoop) { processor.setTransportLooping(shouldLoop); };
    transportBar.onRecord = [this](bool shouldRecord) { processor.setTransportRecording(shouldRecord); };
    transportBar.onTap = [this]
    {
        const auto currentTempo = processor.getMIDIPlayer().getTempo();
        processor.setTransportTempo(static_cast<float>(juce::jlimit(30.0, 300.0, currentTempo + 1.0)));
    };
    transportBar.onMasterVolume = [this](float level)
    {
        if (auto* parameter = processor.getAPVTS().getParameter("masterLevel"))
            parameter->setValueNotifyingHost(parameter->convertTo0to1(level));
    };

    ErrorLogger::getInstance().addListener(this);
    addKeyListener(this);

    switchTab(0);
    startTimer(33);
    resized();
}

DrumTechEditor::~DrumTechEditor()
{
    setLookAndFeel(nullptr);
    ThemeManager::get().onThemeChanged = nullptr;
    ErrorLogger::getInstance().removeListener(this);
    removeKeyListener(this);
}

//==============================================================================
void DrumTechEditor::applyTheme()
{
    auto& tm = ThemeManager::get();
    auto bgCol     = tm.bg();
    auto panelCol  = tm.panel();
    auto textCol2  = tm.text();
    auto mutedCol  = tm.muted();

    // Tab buttons use transparent backgrounds and icons, no color updates needed
    juce::ignoreUnused(panelCol, mutedCol);

    kitSelectorBtn.setColour(juce::TextButton::buttonColourId,  tm.surfaceHi());
    kitSelectorBtn.setColour(juce::TextButton::textColourOffId, textCol2);
    themeModeBtn.setColour  (juce::TextButton::buttonColourId,  tm.surfaceHi());
    themeModeBtn.setColour  (juce::TextButton::textColourOffId, tm.accent());

    settingsView.setCurrentKitName(currentKitName);

    juce::ignoreUnused(bgCol, mutedCol, textCol2);
}

void DrumTechEditor::showKitPopupMenu()
{
    if (kitLocked)
    {
        settingsView.setImportStatus("blocked: unlock kit to import/switch");
        return;
    }

    juce::PopupMenu menu;
    menu.addSectionHeader("Select Kit");
    menu.addItem(1, "Default Kit", true, currentKitName == "Default Kit");
    menu.addItem(2, "Rock Kit",    true, currentKitName == "Rock Kit");
    menu.addItem(3, "Jazz Kit",    true, currentKitName == "Jazz Kit");
    menu.addSeparator();
    menu.addItem(10, "Add New Kit...");
    menu.addItem(11, "Import Folder...");

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&kitSelectorBtn),
        [this](int result)
        {
            if (result == 1)  { currentKitName = "Default Kit"; kitSelectorBtn.setButtonText("Kit: Default"); settingsView.setCurrentKitName(currentKitName); }
            else if (result == 2) { currentKitName = "Rock Kit";    kitSelectorBtn.setButtonText("Kit: Rock"); settingsView.setCurrentKitName(currentKitName); }
            else if (result == 3) { currentKitName = "Jazz Kit";    kitSelectorBtn.setButtonText("Kit: Jazz"); settingsView.setCurrentKitName(currentKitName); }
            else if (result == 10 || result == 11)
            {
                // Folder import
                auto chooser = std::make_shared<juce::FileChooser>(
                    "Choose a folder with drum samples",
                    juce::File::getSpecialLocation(juce::File::userMusicDirectory));
                chooser->launchAsync(juce::FileBrowserComponent::openMode |
                                     juce::FileBrowserComponent::canSelectDirectories,
                    [this, chooser](const juce::FileChooser& fc)
                    {
                        auto folder = fc.getResult();
                        if (folder.isDirectory())
                        {
                            processor.loadSamplesFromFolder(folder);
                            currentKitName = folder.getFileName();
                            kitSelectorBtn.setButtonText("Kit: " + folder.getFileName());
                            settingsView.setCurrentKitName(currentKitName);
                            settingsView.setImportStatus("loaded " + folder.getFileName());
                            LOG_INFO("Loaded kit from folder: " + folder.getFullPathName());
                        }
                        else
                        {
                            settingsView.setImportStatus("cancelled");
                        }
                    });
            }
        });
}

//==============================================================================
void DrumTechEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    
    // Draw black bar at top for icon navigation
    static constexpr int topBarHeight = 60;
    g.setColour(juce::Colours::black);
    g.fillRect(0, 0, getWidth(), topBarHeight);
    
    // Draw icons centered in the black bar
    static constexpr int iconSize = 48;
    if (homeIconImg.isValid())
    {
        auto bounds = tabHome.getBounds();
        g.setColour(juce::Colours::white);
        g.drawImage(homeIconImg, bounds.toFloat(), 
                   juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }
    
    if (kitBuilderIconImg.isValid())
    {
        auto bounds = tabKitBuilder.getBounds();
        g.setColour(juce::Colours::white);
        g.drawImage(kitBuilderIconImg, bounds.toFloat(), 
                   juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }
    
    if (mixerIconImg.isValid())
    {
        auto bounds = tabMixer.getBounds();
        g.setColour(juce::Colours::white);
        g.drawImage(mixerIconImg, bounds.toFloat(), 
                   juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }
    
    if (triggerIconImg.isValid())
    {
        auto bounds = tabTrigger.getBounds();
        g.setColour(juce::Colours::white);
        g.drawImage(triggerIconImg, bounds.toFloat(), 
                   juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }
    
    if (settingsIconImg.isValid())
    {
        auto bounds = tabSettings.getBounds();
        g.setColour(juce::Colours::white);
        g.drawImage(settingsIconImg, bounds.toFloat(), 
                   juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }
    
    if (mixerSlideAmount > 0.001f)
    {
        g.setColour(juce::Colours::black.withAlpha(0.35f * mixerSlideAmount));
        g.fillRect(getLocalBounds());
    }
}

juce::String DrumTechEditor::activeTabContextTitle() const
{
    switch (activeTab)
    {
        case 0:  return "Home";
        case 1:  return "Kit Builder";
        case 2:  return "Mixer";
        case 3:  return "Trigger";
        case 4:  return "Settings";
        default: return {};
    }
}

void DrumTechEditor::paintInstrumentBar(juce::Graphics& g, juce::Rectangle<int> instrBar)
{
    auto& tm = ThemeManager::get();
    g.setColour(tm.panel().darker(0.08f));
    g.fillRect(instrBar);

    // Subtle top highlight (raised edge)
    g.setColour(tm.borderHi().withAlpha(0.35f));
    g.drawHorizontalLine(instrBar.getY(), 0.0f, (float)getWidth());

    g.setColour(tm.border());
    g.drawHorizontalLine(instrBar.getBottom() - 1, 0.0f, (float)getWidth());

    g.setFont(juce::FontOptions(11.5f, juce::Font::plain));
    g.setColour(tm.muted());
    const juce::String ctx = "Current view: " + activeTabContextTitle();
    g.drawText(ctx, instrBar.reduced(16, 0), juce::Justification::centred, true);
}

void DrumTechEditor::paintTabBar(juce::Graphics& g, juce::Rectangle<int>)
{
    // Draw active tab indicator (cyan underline) for the selected icon
    juce::TextButton* tabs[] = { &tabHome, &tabKitBuilder, &tabMixer, &tabTrigger, &tabSettings };
    if (activeTab >= 0 && activeTab < 5)
    {
        auto tb = tabs[activeTab]->getBounds();
        g.setColour(juce::Colour(PluginColors::accent));
        g.fillRect(tb.getX(), tb.getBottom() - 3, tb.getWidth(), 3);
    }
}

//==============================================================================
void DrumTechEditor::resized()
{
    auto area = getLocalBounds();
    
    // Top black bar with centered icon navigation
    static constexpr int topBarHeight = 60;
    static constexpr int iconSize = 48;
    static constexpr int iconGap = 20;
    
    auto topBar = area.removeFromTop(topBarHeight);
    
    // Calculate total width needed for 5 icons with gaps
    int totalIconWidth = (iconSize * 5) + (iconGap * 4);
    int startX = (topBar.getWidth() - totalIconWidth) / 2;
    int iconY = (topBarHeight - iconSize) / 2;
    
    // Position 5 icon buttons horizontally and centered
    tabHome.setBounds(startX, iconY, iconSize, iconSize);
    tabHome.setVisible(true);
    tabHome.toFront(true);
    
    tabKitBuilder.setBounds(startX + (iconSize + iconGap), iconY, iconSize, iconSize);
    tabKitBuilder.setVisible(true);
    tabKitBuilder.toFront(true);
    
    tabMixer.setBounds(startX + (iconSize + iconGap) * 2, iconY, iconSize, iconSize);
    tabMixer.setVisible(true);
    tabMixer.toFront(true);
    
    tabTrigger.setBounds(startX + (iconSize + iconGap) * 3, iconY, iconSize, iconSize);
    tabTrigger.setVisible(true);
    tabTrigger.toFront(true);
    
    tabSettings.setBounds(startX + (iconSize + iconGap) * 4, iconY, iconSize, iconSize);
    tabSettings.setVisible(true);
    tabSettings.toFront(true);
    
    // Ensure tab buttons are on top of everything
    tabHome.toFront(false);
    tabKitBuilder.toFront(false);
    tabMixer.toFront(false);
    tabTrigger.toFront(false);
    tabSettings.toFront(false);
    
    // Hide old controls that were on the side
    kitSelectorBtn.setVisible(false);
    kitLockBtn.setVisible(false);
    themeModeBtn.setVisible(false);
    grooveTimeline.setVisible(false);
    transportBar.setVisible(false);
    grooveBrowser.setVisible(false);

    // Layout all content views to fill the remaining area
    kitView.setBounds(area);
    settingsView.setBounds(area);
    triggerView.setBounds(area);

    // Mixer slides in from the right
    const int mixerW = juce::jlimit(640, area.getWidth() - 120, static_cast<int>(area.getWidth() * 0.72f));
    const int mixerX = area.getRight() - static_cast<int>(mixerSlideAmount * static_cast<float>(mixerW));
    mixerView.setBounds(mixerX, area.getY(), mixerW, area.getHeight());
    mixerView.setVisible((mixerSlideAmount > 0.001f) && !debugConsoleVisible);
    if (mixerView.isVisible())
        mixerView.toFront(false);

    mixerDismissLayer.setVisible((mixerSlideAmount > 0.001f) && !debugConsoleVisible);
    if (mixerDismissLayer.isVisible())
    {
        mixerDismissLayer.setBounds(area.getX(), area.getY(),
                                    juce::jmax(0, mixerX - area.getX()), area.getHeight());
        mixerDismissLayer.toFront(false);
        mixerView.toFront(false);
    }

    if (debugConsole)
    {
        debugConsole->setVisible(debugConsoleVisible);
        if (debugConsoleVisible) debugConsole->setBounds(area);
    }

    repaint();
}

//==============================================================================
void DrumTechEditor::switchTab(int tab)
{
    activeTab = tab;
    
    // Hide all views first
    kitView.setVisible(false);
    settingsView.setVisible(false);
    triggerView.setVisible(false);
    grooveBrowser.setVisible(false);
    
    // Show the selected view
    switch (activeTab)
    {
        case 0:  // Home - show drum kit view
            kitView.setVisible(true);
            kitBuilderSlideTarget = 0.0f;  // Close kit builder panel
            mixerSlideTarget = 0.0f;  // Close mixer if open
            break;
            
        case 1:  // Kit Builder - open kit builder settings panel
            kitView.setVisible(true);
            kitBuilderSlideTarget = 1.0f;  // Slide in kit builder panel
            mixerSlideTarget = 0.0f;  // Close mixer
            break;
            
        case 2:  // Mixer - slide in mixer
            kitView.setVisible(true);
            kitBuilderSlideTarget = 0.0f;  // Close kit builder panel
            mixerSlideTarget = 1.0f;   // Slide mixer in
            break;
            
        case 3:  // Trigger - show trigger view
            triggerView.setVisible(true);
            kitBuilderSlideTarget = 0.0f;
            mixerSlideTarget = 0.0f;
            break;
            
        case 4:  // Settings - show settings view
            settingsView.setVisible(true);
            kitBuilderSlideTarget = 0.0f;
            mixerSlideTarget = 0.0f;
            break;
    }
    
    applyTheme();
    resized();
    repaint();
}

bool DrumTechEditor::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    if (key == juce::KeyPress('d', juce::ModifierKeys::ctrlModifier
                                 | juce::ModifierKeys::shiftModifier, 0))
    {
        toggleDebugConsole();
        return true;
    }
    return false;
}

void DrumTechEditor::toggleDebugConsole()
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
    }
    else if (debugConsole)
    {
        debugConsole->setVisible(false);
    }
    resized();
    repaint();
}

void DrumTechEditor::updateStatus()
{
    int n = processor.getNumLoadedSamples();
    if (n > 0) LOG_INFO("Loaded " + juce::String(n) + " samples");
    else        LOG_WARNING("No samples loaded");
}

void DrumTechEditor::logUpdated() {}
