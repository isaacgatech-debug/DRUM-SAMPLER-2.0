#include "PluginEditor.h"
#include "../UI/ThemeManager.h"

DrumTechEditor::DrumTechEditor(DrumTechProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(1400, 900);
    setResizable(true, true);
    setResizeLimits(1200, 800, 4000, 2000);

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
    styleTab(tabKit,     0, switchFn); addAndMakeVisible(tabKit);
    styleTab(tabMixer,   1, switchFn); addAndMakeVisible(tabMixer);
    styleTab(tabTrigger, 2, switchFn); addAndMakeVisible(tabTrigger);
    styleTab(tabGrooves, 3, switchFn); addAndMakeVisible(tabGrooves);

    // Kit selector (shows popup menu instead of combo)
    kitSelectorBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
    kitSelectorBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textPrimary));
    kitSelectorBtn.onClick = [this] { showKitPopupMenu(); };
    addAndMakeVisible(kitSelectorBtn);

    // Content views
    kitView.setProcessor(&processor);
    mixerView.setProcessor(&processor);
    triggerUI.setAudioTriggerEngine(&processor.getTriggerEngine());
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
    addAndMakeVisible(grooveBrowser);
    addAndMakeVisible(mixerView);
    addAndMakeVisible(triggerUI);

    // MIDI roll + transport (always visible)
    addAndMakeVisible(grooveTimeline);
    addAndMakeVisible(transportBar);
    transportBar.onPlay = [this](bool shouldPlay) { processor.setTransportPlaying(shouldPlay); };
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
    juce::TextButton* tabs[] = { &tabKit, &tabMixer, &tabTrigger, &tabGrooves };
    for (int i = 0; i < 4; ++i)
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

    juce::ignoreUnused(bgCol, mutedCol, textCol2);
}

void DrumTechEditor::showKitPopupMenu()
{
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
            if (result == 1)  { currentKitName = "Default Kit"; kitSelectorBtn.setButtonText("Kit: Default"); }
            else if (result == 2) { currentKitName = "Rock Kit";    kitSelectorBtn.setButtonText("Kit: Rock"); }
            else if (result == 3) { currentKitName = "Jazz Kit";    kitSelectorBtn.setButtonText("Kit: Jazz"); }
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
                            LOG_INFO("Loaded kit from folder: " + folder.getFullPathName());
                        }
                    });
            }
        });
}

//==============================================================================
void DrumTechEditor::paint(juce::Graphics& g)
{
    auto& tm = ThemeManager::get();

    g.fillAll(tm.bg());

    // Top navigation bar
    juce::Rectangle<int> topNav(0, 0, getWidth(), topNavH);
    g.setColour(tm.panel());
    g.fillRect(topNav);

    // Bottom border on top nav
    g.setColour(tm.border());
    g.drawHorizontalLine(topNavH - 1, 0.0f, (float)getWidth());

    // Logo (22x22)
    if (logoImage.isValid())
    {
        g.drawImage(logoImage,
                    juce::Rectangle<float>(10.0f, (topNavH - 22.0f) * 0.5f, 22.0f, 22.0f),
                    juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }
    else
    {
        // Placeholder 'G' badge
        g.setColour(juce::Colour(PluginColors::accent));
        g.fillEllipse(10, (topNavH - 22) / 2, 22, 22);
        g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        g.setColour(juce::Colours::black);
        g.drawText("G", 10, (topNavH - 22) / 2, 22, 22, juce::Justification::centred, false);
    }

    // ── Branding ──────────────────────────────────────────────────────────
    float tx = 40.0f;
    float ty = (topNavH - 20.0f) * 0.5f;

    // "GRIDLOCK" — white bold
    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    g.setColour(tm.text());
    g.drawText("GRIDLOCK", (int)tx, (int)ty, 90, 20, juce::Justification::centredLeft, false);

    // Separator
    g.setFont(PluginFonts::mono(14.0f));
    g.setColour(tm.border());
    g.drawText("|", (int)(tx + 93), (int)ty, 14, 20, juce::Justification::centred, false);

    // "DRUM TECH" — stylized: italic bold, cyan, letterSpacing via char-by-char
    {
        const juce::String title = "DRUM TECH";
        juce::Font titleFont(juce::FontOptions(15.0f, juce::Font::bold | juce::Font::italic));
        g.setFont(titleFont);

        float charX = tx + 110.0f;
        float spacing = 1.5f;  // extra inter-character gap
        juce::Colour c1(PluginColors::accent);
        juce::Colour c2(0xFF80E8FF);

        for (int i = 0; i < title.length(); ++i)
        {
            float t = (float)i / (float)(title.length() - 1);
            g.setColour(c1.interpolatedWith(c2, t));
            juce::String ch(title.substring(i, i + 1));
            float cw = titleFont.getStringWidthFloat(ch);
            g.drawText(ch, (int)charX, (int)ty, (int)(cw + spacing + 2), 20,
                       juce::Justification::centredLeft, false);
            charX += cw + spacing;
        }
    }

    // Instrument bar
    juce::Rectangle<int> instrBar(0, topNavH, getWidth(), instrBarH);
    g.setColour(tm.panel().darker(0.06f));
    g.fillRect(instrBar);
    g.setColour(tm.border());
    g.drawHorizontalLine(topNavH + instrBarH - 1, 0.0f, (float)getWidth());

    // Tab underlines (painted over buttons)
    paintTabBar(g, {0, 0, getWidth(), topNavH});
}

void DrumTechEditor::paintTabBar(juce::Graphics& g, juce::Rectangle<int>)
{
    juce::TextButton* tabs[] = { &tabKit, &tabMixer, &tabTrigger, &tabGrooves };
    for (int i = 0; i < 4; ++i)
    {
        if (i == activeTab)
        {
            auto tb = tabs[i]->getBounds();
            // Cyan underline (3px)
            g.setColour(juce::Colour(PluginColors::accent));
            g.fillRect(tb.getX(), tb.getBottom() - 3, tb.getWidth(), 3);
            // Subtle tint
            g.setColour(juce::Colour(PluginColors::accent).withAlpha(0.07f));
            g.fillRect(tb);
        }
    }
}

//==============================================================================
void DrumTechEditor::resized()
{
    auto area = getLocalBounds();
    auto topNavArea = area.removeFromTop(topNavH);

    // Tab buttons
    int tabX = 240, tabW = 90;
    tabKit    .setBounds(tabX,            0, tabW, topNavH);
    tabMixer  .setBounds(tabX + tabW,     0, tabW, topNavH);
    tabTrigger.setBounds(tabX + tabW * 2, 0, tabW, topNavH);
    tabGrooves.setBounds(tabX + tabW * 3, 0, tabW, topNavH);

    // Right-side controls in top nav
    int rightX = getWidth() - 4;
    themeModeBtn.setBounds(rightX - 60,  (topNavH - 24) / 2, 58, 24);
    rightX -= 64;
    kitSelectorBtn.setBounds(rightX - 130, (topNavH - 24) / 2, 128, 24);

    // Instrument bar
    area.removeFromTop(instrBarH);

    // Bottom: transport + MIDI roll
    auto transportArea = area.removeFromBottom(transportBarH);
    auto rollArea      = area.removeFromBottom(midiRollH);
    transportBar  .setBounds(transportArea);
    grooveTimeline.setBounds(rollArea);

    // Content area
    kitView      .setBounds(area);
    grooveBrowser.setBounds(area);
    mixerView    .setBounds(area);
    triggerUI    .setBounds(area);

    kitView      .setVisible(activeTab == 0 && !debugConsoleVisible);
    grooveBrowser.setVisible(activeTab == 3 && !debugConsoleVisible);
    mixerView    .setVisible(activeTab == 1 && !debugConsoleVisible);
    triggerUI    .setVisible(activeTab == 2 && !debugConsoleVisible);

    if (debugConsole)
    {
        debugConsole->setVisible(debugConsoleVisible);
        if (debugConsoleVisible) debugConsole->setBounds(area);
    }

    repaint();
    juce::ignoreUnused(topNavArea);
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
