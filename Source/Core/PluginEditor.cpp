#include "PluginEditor.h"
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

    // Tab buttons
    auto styleTab = [](juce::TextButton& btn, int tabIdx,
                       std::function<void(int)> switchFn)
    {
        btn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginPanel));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textMuted));
        btn.setColour(juce::TextButton::textColourOnId,  juce::Colour(PluginColors::accent));
        btn.onClick = [tabIdx, switchFn] { switchFn(tabIdx); };
    };

    auto switchFn = [this](int t) { switchTab(t); };
    styleTab(tabKit,      0, switchFn); addAndMakeVisible(tabKit);
    styleTab(tabMixer,    1, switchFn); addAndMakeVisible(tabMixer);
    styleTab(tabSettings, 2, switchFn); addAndMakeVisible(tabSettings);

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

    // Update tab colours
    juce::TextButton* tabs[] = { &tabKit, &tabMixer, &tabSettings };
    for (int i = 0; i < 3; ++i)
    {
        bool active = (i == activeTab);
        tabs[i]->setColour(juce::TextButton::buttonColourId,  panelCol);
        tabs[i]->setColour(juce::TextButton::textColourOffId,
            active ? juce::Colour(PluginColors::accent) : mutedCol);
    }

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
        case 1:  return "Mix";
        case 2:  return "Settings";
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
    auto& tm = ThemeManager::get();
    juce::TextButton* tabs[] = { &tabKit, &tabMixer, &tabSettings };
    for (int i = 0; i < 3; ++i)
    {
        auto tb = tabs[i]->getBounds();
        if (i > 0)
        {
            g.setColour(tm.border().withAlpha(0.85f));
            g.drawVerticalLine(tb.getX(), (float)tb.getY() + 6.0f, (float)tb.getBottom() - 6.0f);
        }

        if (i == activeTab)
        {
            g.setColour(juce::Colour(PluginColors::accent));
            g.fillRect(tb.getX(), tb.getBottom() - 2, tb.getWidth(), 2);
        }
    }
}

//==============================================================================
void DrumTechEditor::resized()
{
    // Immersive single-screen mode driven by backdrop hotspots.
    auto area = getLocalBounds();

    tabKit.setVisible(false);
    tabMixer.setVisible(false);
    tabSettings.setVisible(false);
    kitSelectorBtn.setVisible(false);
    kitLockBtn.setVisible(false);
    themeModeBtn.setVisible(false);
    grooveTimeline.setVisible(false);
    transportBar.setVisible(false);
    settingsView.setVisible(false);
    grooveBrowser.setVisible(false);

    kitView.setVisible(!debugConsoleVisible);
    kitView.setBounds(area);

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
    applyTheme();
    resized();
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
