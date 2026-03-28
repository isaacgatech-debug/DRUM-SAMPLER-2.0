#include "TriggerUI.h"
#include <juce_audio_formats/juce_audio_formats.h>

const char* TriggerUI::micNames[8] = {
    "Kick In", "Kick Out", "Snare Top", "Snare Bot",
    "Hi-Hat", "Tom 1", "Tom 2", "Tom 3"
};

//==============================================================================
// TriggerChannel
//==============================================================================
TriggerUI::TriggerChannel::TriggerChannel(const juce::String& micName)
    : channelName(micName)
{
    // Assign a channel color based on name order
    static int colorIdx = 0;
    int ci = colorIdx++ % 12;
    chColor = PluginColors::channelColors[ci];

    // Import
    importBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
    importBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::accent));
    importBtn.onClick = [this]
    {
        auto chooser = std::make_shared<juce::FileChooser>(
            "Select audio file", juce::File(), "*.wav;*.aif;*.aiff;*.mp3");
        chooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f.existsAsFile()) loadAudioFile(f);
            });
    };
    addAndMakeVisible(importBtn);

    // Clear
    clearBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
    clearBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::meterRed));
    clearBtn.onClick = [this]
    {
        audioBuffer.setSize(0, 0);
        filenameLabel.setText("", juce::dontSendNotification);
        repaint();
    };
    addAndMakeVisible(clearBtn);

    // Play
    playBtn.setClickingTogglesState(true);
    playBtn.setColour(juce::TextButton::buttonColourId,   juce::Colour(PluginColors::pluginSurface));
    playBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(PluginColors::accent));
    playBtn.setColour(juce::TextButton::textColourOffId,  juce::Colour(PluginColors::textPrimary));
    playBtn.onClick = [this]
    {
        if (audioBuffer.getNumSamples() == 0 || triggerEngine == nullptr)
        {
            playBtn.setToggleState(false, juce::dontSendNotification);
            return;
        }

        if (playBtn.getToggleState())
        {
            const auto tempFile = juce::File::createTempFile(".wav");
            {
                juce::WavAudioFormat wav;
                std::unique_ptr<juce::AudioFormatWriter> writer(
                    wav.createWriterFor(new juce::FileOutputStream(tempFile),
                                        audioSampleRate,
                                        static_cast<unsigned int>(audioBuffer.getNumChannels()),
                                        16, {}, 0));
                if (writer != nullptr)
                    writer->writeFromAudioSampleBuffer(audioBuffer, 0, audioBuffer.getNumSamples());
            }
            triggerPreviewResults.clear();
            triggerEngine->processAudioFile(tempFile, triggerPreviewResults);
            tempFile.deleteFile();
        }
    };
    addAndMakeVisible(playBtn);

    // Expand / EQ toggle
    expandBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
    expandBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::textMuted));
    expandBtn.onClick = [this]
    {
        freqEditorVisible = !freqEditorVisible;
        expandBtn.setButtonText(freqEditorVisible ? "∧ EQ" : "∨ EQ");
        freqEditor.setVisible(freqEditorVisible);
        // Notify parent to re-layout
        if (auto* p = getParentComponent()) p->resized();
    };
    addAndMakeVisible(expandBtn);

    // Filename label
    filenameLabel.setText("Drop or Import audio", juce::dontSendNotification);
    filenameLabel.setFont(PluginFonts::mono(9.0f));
    filenameLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textMuted));
    filenameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(filenameLabel);

    // Threshold slider
    thresholdSlider.setRange(0.0, 1.0, 0.01);
    thresholdSlider.setValue(0.3);
    thresholdSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 42, 16);
    thresholdSlider.setColour(juce::Slider::thumbColourId,             chColor);
    thresholdSlider.setColour(juce::Slider::trackColourId,             chColor.withAlpha(0.4f));
    thresholdSlider.setColour(juce::Slider::backgroundColourId,        juce::Colour(PluginColors::pluginBg));
    thresholdSlider.setColour(juce::Slider::textBoxTextColourId,       juce::Colour(PluginColors::textPrimary));
    thresholdSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(PluginColors::pluginSurface));
    thresholdSlider.setColour(juce::Slider::textBoxOutlineColourId,    juce::Colour(PluginColors::pluginBorder));
    thresholdSlider.onValueChange = [this]
    {
        if (triggerEngine != nullptr)
            triggerEngine->setThreshold(static_cast<float>(thresholdSlider.getValue()));
    };
    addAndMakeVisible(thresholdSlider);

    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.setFont(PluginFonts::label(8.5f));
    thresholdLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textMuted));
    addAndMakeVisible(thresholdLabel);

    // FreqEditor (hidden by default)
    freqEditor.setVisible(false);
    addAndMakeVisible(freqEditor);

    setMouseCursor(juce::MouseCursor::NormalCursor);
}

//==============================================================================
void TriggerUI::TriggerChannel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Row background
    g.setColour(juce::Colour(PluginColors::pluginSurface));
    g.fillRect(bounds);

    // Bottom border
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawHorizontalLine(bounds.getBottom() - 1, 0.0f, (float)bounds.getRight());

    // Left color accent bar (3px)
    g.setColour(chColor);
    g.fillRect(0, 0, 3, bounds.getHeight());

    // Draw left panel info
    auto leftArea = bounds.removeFromLeft(leftW);
    drawLeftPanel(g, leftArea);

    // Waveform area (fills remaining, minus threshold row at bottom)
    auto wfRow = bounds.removeFromTop(rowH - 28);
    drawWaveform(g, wfRow);
}

void TriggerUI::TriggerChannel::drawLeftPanel(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour(juce::Colour(PluginColors::pluginPanel));
    g.fillRect(area);

    // Right border of left panel
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawVerticalLine(area.getRight() - 1, (float)area.getY(), (float)area.getBottom());

    // Channel name
    g.setFont(PluginFonts::label(10.0f));
    g.setColour(chColor);
    g.drawText(channelName, area.reduced(6, 4).removeFromTop(18),
               juce::Justification::centredLeft, false);
}

void TriggerUI::TriggerChannel::drawWaveform(juce::Graphics& g, juce::Rectangle<int> area)
{
    // Waveform background
    g.setColour(juce::Colour(PluginColors::pluginBg));
    g.fillRect(area);

    if (isDragOver)
    {
        g.setColour(juce::Colour(PluginColors::accent).withAlpha(0.2f));
        g.fillRect(area);
        g.setColour(juce::Colour(PluginColors::accent));
        g.drawRect(area, 1);
        g.setFont(PluginFonts::label(10.0f));
        g.setColour(juce::Colour(PluginColors::accent));
        g.drawText("Drop audio here", area, juce::Justification::centred, false);
        return;
    }

    if (audioBuffer.getNumSamples() == 0)
    {
        g.setFont(PluginFonts::label(9.0f));
        g.setColour(juce::Colour(PluginColors::textMuted).withAlpha(0.5f));
        g.drawText("Import or drag an audio file to see waveform", area,
                   juce::Justification::centred, false);
        return;
    }

    int numSamples  = audioBuffer.getNumSamples();
    int w           = area.getWidth();
    float h         = (float)area.getHeight();
    float centerY   = area.getY() + h * 0.5f;

    // Centre line
    g.setColour(chColor.withAlpha(0.15f));
    g.drawHorizontalLine((int)centerY, (float)area.getX(), (float)area.getRight());

    // Build waveform path — top and bottom (mirrored)
    juce::Path top, bot;
    bool firstTop = true, firstBot = true;
    int samplesPerPixel = juce::jmax(1, numSamples / w);

    for (int px = 0; px < w; ++px)
    {
        int start = (px * numSamples) / w;
        int end   = juce::jmin(start + samplesPerPixel, numSamples);

        float maxSample = 0.0f, minSample = 0.0f;
        for (int ch = 0; ch < audioBuffer.getNumChannels(); ++ch)
        {
            const float* data = audioBuffer.getReadPointer(ch);
            for (int s = start; s < end; ++s)
            {
                maxSample = juce::jmax(maxSample, data[s]);
                minSample = juce::jmin(minSample, data[s]);
            }
        }

        float xPos    = (float)(area.getX() + px);
        float topY    = centerY - maxSample * (h * 0.47f);
        float botY    = centerY - minSample * (h * 0.47f);

        topY = juce::jlimit((float)area.getY(), centerY, topY);
        botY = juce::jlimit(centerY, (float)area.getBottom(), botY);

        if (firstTop) { top.startNewSubPath(xPos, topY); firstTop = false; }
        else top.lineTo(xPos, topY);
        if (firstBot) { bot.startNewSubPath(xPos, botY); firstBot = false; }
        else bot.lineTo(xPos, botY);
    }

    // Fill between top and bot paths
    juce::Path filled = top;
    for (int px = w - 1; px >= 0; --px)
    {
        // Reverse the bot path to create closed shape — approximate via rect fill per pixel
        juce::ignoreUnused(px);
    }
    // Use stroke paths instead (cleaner)
    juce::ColourGradient wfGrad(chColor.withAlpha(0.7f), (float)area.getX(), centerY - h * 0.4f,
                                 chColor.withAlpha(0.2f), (float)area.getX(), centerY, false);
    g.setGradientFill(wfGrad);
    g.strokePath(top, juce::PathStrokeType(1.5f));

    juce::ColourGradient wfGrad2(chColor.withAlpha(0.2f), (float)area.getX(), centerY,
                                  chColor.withAlpha(0.7f), (float)area.getX(), centerY + h * 0.4f, false);
    g.setGradientFill(wfGrad2);
    g.strokePath(bot, juce::PathStrokeType(1.5f));

    // Filled area between the two paths using a simple approach
    juce::Path fill;
    fill.addPath(top);
    fill.lineTo((float)area.getRight(), centerY);
    fill.lineTo((float)area.getX(), centerY);
    fill.closeSubPath();
    g.setColour(chColor.withAlpha(0.12f));
    g.fillPath(fill);
}

//==============================================================================
void TriggerUI::TriggerChannel::resized()
{
    auto area = getLocalBounds();
    auto left = area.removeFromLeft(leftW);

    // Left panel buttons
    left.removeFromTop(22);   // channel name text area
    importBtn.setBounds(left.removeFromTop(22).reduced(4, 2));
    clearBtn .setBounds(left.removeFromTop(22).reduced(4, 2));
    playBtn  .setBounds(left.removeFromTop(22).reduced(4, 2));
    expandBtn.setBounds(left.removeFromTop(22).reduced(4, 2));
    filenameLabel.setBounds(left.reduced(4, 2));

    // Right side: waveform area (top portion), threshold row, then optional freq editor
    int thrH = 26;
    auto thrRow = area.removeFromBottom(thrH);
    thresholdLabel.setBounds(thrRow.removeFromLeft(70));
    thresholdSlider.setBounds(thrRow);

    // Waveform area (fills)
    area.removeFromTop(2);
    // waveform is drawn in paint() — no child component needed

    // Freq editor
    if (freqEditorVisible)
        freqEditor.setBounds(area.removeFromBottom(freqEditorVisible ? 130 : 0));
    else
        freqEditor.setBounds(juce::Rectangle<int>());
}

//==============================================================================
void TriggerUI::TriggerChannel::loadAudioFile(const juce::File& file)
{
    juce::AudioFormatManager fmt;
    fmt.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(fmt.createReaderFor(file));

    if (reader)
    {
        int len = static_cast<int>(reader->lengthInSamples);
        audioBuffer.setSize(static_cast<int>(reader->numChannels), len);
        reader->read(&audioBuffer, 0, len, 0, true, true);
        audioSampleRate = reader->sampleRate;
        filenameLabel.setText(file.getFileNameWithoutExtension(),
                              juce::dontSendNotification);
        repaint();
    }
}

//==============================================================================
bool TriggerUI::TriggerChannel::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
        if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".aif") ||
            f.endsWithIgnoreCase(".aiff") || f.endsWithIgnoreCase(".mp3"))
            return true;
    return false;
}

void TriggerUI::TriggerChannel::fileDragEnter(const juce::StringArray& files, int, int)
{
    if (isInterestedInFileDrag(files)) { isDragOver = true; repaint(); }
}

void TriggerUI::TriggerChannel::fileDragExit(const juce::StringArray&)
{
    isDragOver = false; repaint();
}

void TriggerUI::TriggerChannel::filesDropped(const juce::StringArray& files, int, int)
{
    isDragOver = false;
    if (!files.isEmpty()) loadAudioFile(juce::File(files[0]));
}

//==============================================================================
// TriggerUI
//==============================================================================
TriggerUI::TriggerUI()
{
    scrollView.setViewedComponent(&channelsContainer, false);
    scrollView.setScrollBarsShown(true, false);   // vertical scroll only
    addAndMakeVisible(scrollView);

    for (int i = 0; i < 8; ++i)
    {
        channels[i] = std::make_unique<TriggerChannel>(micNames[i]);
        channelsContainer.addAndMakeVisible(*channels[i]);
    }
}

void TriggerUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(PluginColors::pluginBg));

    // Header bar
    g.setColour(juce::Colour(PluginColors::pluginPanel));
    g.fillRect(0, 0, getWidth(), 28);
    g.setFont(PluginFonts::label(10.0f));
    g.setColour(juce::Colour(PluginColors::textMuted));
    g.drawText("TRIGGER CHANNELS — Import audio per mic, drag waveforms to adjust",
               10, 0, getWidth() - 20, 28, juce::Justification::centredLeft, false);
}

void TriggerUI::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(28);  // header
    scrollView.setBounds(area);

    int margin = 4;
    int totalH = 0;

    for (int i = 0; i < 8; ++i)
    {
        int rowH = channels[i]->isExpanded() ? TriggerChannel::expandedH : TriggerChannel::rowH;
        channels[i]->setBounds(margin, totalH + margin,
                               area.getWidth() - margin * 2, rowH);
        totalH += rowH + margin;
    }

    channelsContainer.setSize(area.getWidth(), totalH + margin);
}

bool TriggerUI::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
        if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".aif") ||
            f.endsWithIgnoreCase(".aiff") || f.endsWithIgnoreCase(".mp3"))
            return true;
    return false;
}

void TriggerUI::setAudioTriggerEngine(AudioTriggerEngine* engine)
{
    triggerEngine = engine;
    for (auto& channel : channels)
    {
        if (channel)
            channel->setTriggerEngine(engine);
    }
}

void TriggerUI::fileDragEnter(const juce::StringArray& files, int x, int y)
{
    for (auto& ch : channels)
        if (ch && ch->getBounds().contains(x, y))
            ch->fileDragEnter(files, x - ch->getX(), y - ch->getY());
}

void TriggerUI::fileDragExit(const juce::StringArray& files)
{
    for (auto& ch : channels) if (ch) ch->fileDragExit(files);
}

void TriggerUI::filesDropped(const juce::StringArray& files, int x, int y)
{
    for (auto& ch : channels)
        if (ch && ch->getBounds().contains(x, y))
        {
            ch->filesDropped(files, x - ch->getX(), y - ch->getY());
            return;
        }
}
