#include "PluginEditor.h"

// ─── KitView ─────────────────────────────────────────────────────────────────

const KitView::Zone KitView::zones[] = {
    { "Kick",   36, 0.38f, 0.76f, 0.16f, 0.12f, false },
    { "Snare",  38, 0.24f, 0.62f, 0.13f, 0.10f, false },
    { "HH",     42, 0.13f, 0.46f, 0.10f, 0.07f, true  },
    { "Tom1",   50, 0.20f, 0.34f, 0.10f, 0.09f, false },
    { "Tom2",   48, 0.31f, 0.37f, 0.10f, 0.09f, false },
    { "Tom3",   45, 0.43f, 0.38f, 0.10f, 0.09f, false },
    { "Tom4",   43, 0.55f, 0.43f, 0.12f, 0.09f, false },
    { "Crash1", 49, 0.10f, 0.24f, 0.13f, 0.05f, true  },
    { "Ride",   51, 0.60f, 0.26f, 0.14f, 0.05f, true  },
    { "Splash", 55, 0.47f, 0.20f, 0.09f, 0.04f, true  },
    { "Crash2", 57, 0.72f, 0.28f, 0.13f, 0.05f, true  },
};
const int KitView::numZones = 11;

KitView::KitView() {}

void KitView::setFlash (int note)
{
    flashNote  = note;
    flashAlpha = 1.0f;
    repaint();
}

void KitView::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xFF1A1A1A));
    g.fillAll();

    // Drum shell background
    g.setColour (juce::Colour (0xFF252525));
    g.fillEllipse (b.withSizeKeepingCentre (b.getWidth() * 0.7f, b.getHeight() * 0.55f)
                    .translated (0, b.getHeight() * 0.1f));

    for (int i = 0; i < numZones; ++i)
    {
        auto& z    = zones[i];
        float x    = z.x * b.getWidth();
        float y    = z.y * b.getHeight();
        float w    = z.w * b.getWidth();
        float h    = z.h * b.getHeight();
        bool flash = (flashNote == z.note);
        float a    = flash ? (0.5f + flashAlpha * 0.5f) : 0.25f;

        juce::Colour col = z.isCymbal
            ? juce::Colour (0xFFD4A800).withAlpha (a)
            : juce::Colour (0xFFE8A020).withAlpha (a);

        g.setColour (col);
        g.fillEllipse (x, y, w, h);

        g.setColour (col.brighter (0.4f));
        g.drawEllipse (x, y, w, h, 1.5f);

        g.setColour (juce::Colour (0xFFCCCCCC));
        g.setFont (juce::FontOptions ("Helvetica", 10.0f, juce::Font::bold));
        g.drawText (z.name, (int)x, (int)(y + h + 2), (int)w, 14,
                    juce::Justification::centred, false);
    }
}

// ─── DrumLibraryEditor ───────────────────────────────────────────────────────

DrumLibraryEditor::DrumLibraryEditor (DrumLibraryProcessor& p)
    : AudioProcessorEditor (p), processor (p)
{
    setSize (900, 580);
    setResizable (true, true);
    setResizeLimits (600, 400, 1400, 900);

    // Title
    titleLabel.setText ("DRUM LIBRARY", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions ("Helvetica", 20.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, accent);
    addAndMakeVisible (titleLabel);

    // Load button
    loadButton.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xFF2A2A2A));
    loadButton.setColour (juce::TextButton::textColourOffId,  accent);
    loadButton.setColour (juce::TextButton::buttonOnColourId, accent);
    loadButton.onClick = [this] { loadSamplesClicked(); };
    addAndMakeVisible (loadButton);

    // Status
    statusLabel.setFont (juce::FontOptions (12.0f));
    statusLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF888888));
    statusLabel.setText ("No samples loaded — click LOAD SAMPLES to begin.",
                         juce::dontSendNotification);
    addAndMakeVisible (statusLabel);

    // Tabs
    for (auto* btn : { &tabKit, &tabMixer, &tabFX })
    {
        btn->setColour (juce::TextButton::buttonColourId,  juce::Colour (0xFF222222));
        btn->setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF888888));
        btn->setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xFF333333));
        btn->setColour (juce::TextButton::textColourOnId,  accent);
        btn->setClickingTogglesState (true);
        btn->setRadioGroupId (1);
        addAndMakeVisible (btn);
    }
    tabKit.setToggleState (true, juce::dontSendNotification);

    tabKit.onClick   = [this] { activeTab = 0; resized(); repaint(); };
    tabMixer.onClick = [this] { activeTab = 1; resized(); repaint(); };
    tabFX.onClick    = [this] { activeTab = 2; resized(); repaint(); };

    addAndMakeVisible (kitView);

    updateStatus();
    startTimerHz (20);
}

DrumLibraryEditor::~DrumLibraryEditor()
{
    stopTimer();
}

void DrumLibraryEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);

    // Header bar
    g.setColour (header);
    g.fillRect (0, 0, getWidth(), 56);

    // Bottom border of header
    g.setColour (accent.withAlpha (0.4f));
    g.fillRect (0, 56, getWidth(), 1);

    // Tab bar bg
    g.setColour (juce::Colour (0xFF161616));
    g.fillRect (0, 57, getWidth(), 36);

    // Placeholder text for non-kit tabs
    if (activeTab == 1)
    {
        g.setColour (juce::Colour (0xFF555555));
        g.setFont (16.0f);
        g.drawText ("MIXER — coming soon", getLocalBounds(), juce::Justification::centred);
    }
    else if (activeTab == 2)
    {
        g.setColour (juce::Colour (0xFF555555));
        g.setFont (16.0f);
        g.drawText ("FX — coming soon", getLocalBounds(), juce::Justification::centred);
    }
}

void DrumLibraryEditor::resized()
{
    int w = getWidth();

    titleLabel.setBounds (16, 10, 220, 36);
    loadButton.setBounds (w - 170, 12, 150, 32);
    statusLabel.setBounds (240, 15, w - 420, 28);

    int tabY = 58;
    tabKit.setBounds   (0,   tabY, 100, 34);
    tabMixer.setBounds (100, tabY, 100, 34);
    tabFX.setBounds    (200, tabY, 100, 34);

    int contentY = 93;
    kitView.setBounds (0, contentY, w, getHeight() - contentY);
    kitView.setVisible (activeTab == 0);
}

void DrumLibraryEditor::timerCallback()
{
    // Nothing needed — flash is driven by KitView's own repaint
}

void DrumLibraryEditor::loadSamplesClicked()
{
    auto chooser = std::make_shared<juce::FileChooser> ("Select Samples Folder",
                                juce::File::getSpecialLocation (juce::File::userHomeDirectory));
    chooser->launchAsync (juce::FileBrowserComponent::openMode |
                          juce::FileBrowserComponent::canSelectDirectories,
                          [this, chooser] (const juce::FileChooser& fc)
                          {
                              auto folder = fc.getResult();
                              if (folder.isDirectory())
                              {
                                  processor.loadSamplesFromFolder (folder);
                                  updateStatus();
                              }
                          });
}

void DrumLibraryEditor::updateStatus()
{
    int n = processor.getNumLoadedSamples();
    if (n == 0)
        statusLabel.setText ("No samples loaded.", juce::dontSendNotification);
    else
        statusLabel.setText (juce::String (n) + " samples loaded from: " +
                             processor.getLastLoadedFolder().getFileName(),
                             juce::dontSendNotification);
}
