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
    // Import button
    importBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
    importBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::accent));
    importBtn.onClick = [this]
    {
        auto chooser = std::make_shared<juce::FileChooser>("Select audio file", juce::File(),
                                                           "*.wav;*.aif;*.aiff;*.mp3");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f.existsAsFile()) loadAudioFile(f);
            });
    };
    addAndMakeVisible(importBtn);

    // Clear button
    clearBtn.setColour(juce::TextButton::buttonColourId,  juce::Colour(PluginColors::pluginSurface));
    clearBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(PluginColors::meterRed));
    clearBtn.onClick = [this]
    {
        audioBuffer.setSize(0, 0);
        filenameLabel.setText("", juce::dontSendNotification);
        repaint();
    };
    addAndMakeVisible(clearBtn);

    // Play button
    playBtn.setClickingTogglesState(true);
    playBtn.setColour(juce::TextButton::buttonColourId,   juce::Colour(PluginColors::pluginSurface));
    playBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(PluginColors::accent));
    playBtn.setColour(juce::TextButton::textColourOffId,  juce::Colour(PluginColors::textPrimary));
    addAndMakeVisible(playBtn);

    // Filename label
    filenameLabel.setText("No file loaded", juce::dontSendNotification);
    filenameLabel.setFont(PluginFonts::mono(9.0f));
    filenameLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textMuted));
    addAndMakeVisible(filenameLabel);

    // Threshold slider
    thresholdSlider.setRange(0.0, 1.0, 0.01);
    thresholdSlider.setValue(0.3);
    thresholdSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 42, 16);
    thresholdSlider.setColour(juce::Slider::thumbColourId,           juce::Colour(PluginColors::accent));
    thresholdSlider.setColour(juce::Slider::trackColourId,           juce::Colour(PluginColors::accentDim));
    thresholdSlider.setColour(juce::Slider::backgroundColourId,      juce::Colour(PluginColors::pluginBg));
    thresholdSlider.setColour(juce::Slider::textBoxTextColourId,     juce::Colour(PluginColors::textPrimary));
    thresholdSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(PluginColors::pluginSurface));
    thresholdSlider.setColour(juce::Slider::textBoxOutlineColourId,  juce::Colour(PluginColors::pluginBorder));
    addAndMakeVisible(thresholdSlider);

    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.setFont(PluginFonts::label(9.0f));
    thresholdLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::textMuted));
    addAndMakeVisible(thresholdLabel);

    // Frequency band editor
    addAndMakeVisible(freqEditor);
}

void TriggerUI::TriggerChannel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Card background
    g.setColour(juce::Colour(PluginColors::pluginSurface));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    // Header area
    auto headerBounds = bounds.removeFromTop(28.0f);
    g.setColour(juce::Colour(PluginColors::pluginPanel));
    g.fillRoundedRectangle(headerBounds, 4.0f);

    g.setFont(PluginFonts::label(10.0f));
    g.setColour(juce::Colour(PluginColors::textPrimary));
    g.drawText(channelName, headerBounds.reduced(6.0f, 0.0f), juce::Justification::centredLeft, false);

    // Waveform area (72px from top after header)
    auto waveArea = getLocalBounds();
    waveArea.removeFromTop(28 + 4);
    auto wfBounds = waveArea.removeFromTop(72);

    if (isDragOver)
    {
        g.setColour(juce::Colour(PluginColors::accent).withAlpha(0.2f));
        g.fillRect(wfBounds);
        g.setColour(juce::Colour(PluginColors::accent));
        g.drawRect(wfBounds, 1);
        g.setFont(PluginFonts::label(10.0f));
        g.drawText("Drop audio here", wfBounds, juce::Justification::centred, false);
    }
    else if (audioBuffer.getNumSamples() > 0)
    {
        g.setColour(juce::Colour(PluginColors::pluginBg));
        g.fillRect(wfBounds);
        g.setColour(juce::Colour(PluginColors::pluginBorder));
        g.drawRect(wfBounds, 1);
        drawWaveform(g, wfBounds);
    }
    else
    {
        g.setColour(juce::Colour(PluginColors::pluginBg));
        g.fillRect(wfBounds);
        g.setColour(juce::Colour(PluginColors::pluginBorder));
        g.drawRect(wfBounds, 1);
        g.setFont(PluginFonts::label(9.0f));
        g.setColour(juce::Colour(PluginColors::textMuted));
        g.drawText("Drop audio here or click Import", wfBounds, juce::Justification::centred, false);
    }
}

void TriggerUI::TriggerChannel::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(4);

    // Header row (buttons positioned in header)
    auto header = area.removeFromTop(28);
    clearBtn.setBounds(header.removeFromRight(52).reduced(2));
    importBtn.setBounds(header.removeFromRight(90).reduced(2));

    area.removeFromTop(4);
    // Waveform (72px — painted, not a component)
    area.removeFromTop(72);
    area.removeFromTop(4);

    // Playback transport row
    auto transportRow = area.removeFromTop(22);
    playBtn.setBounds(transportRow.removeFromLeft(28).reduced(2));
    filenameLabel.setBounds(transportRow.reduced(2, 0));

    area.removeFromTop(4);

    // Threshold row
    auto thrRow = area.removeFromTop(22);
    thresholdLabel.setBounds(thrRow.removeFromLeft(70));
    thresholdSlider.setBounds(thrRow);

    area.removeFromTop(4);

    // FrequencyBandEditor fills remaining space
    freqEditor.setBounds(area);
}

void TriggerUI::TriggerChannel::drawWaveform(juce::Graphics& g, juce::Rectangle<int> area)
{
    if (audioBuffer.getNumSamples() == 0 || area.getWidth() <= 0) return;

    int numSamples = audioBuffer.getNumSamples();
    int width  = area.getWidth();
    float height = static_cast<float>(area.getHeight());
    float centerY = area.getY() + height / 2.0f;

    g.setColour(juce::Colour(PluginColors::pluginBorder).withAlpha(0.3f));
    g.drawHorizontalLine(static_cast<int>(centerY),
                         static_cast<float>(area.getX()), static_cast<float>(area.getRight()));

    juce::Path waveform;
    bool first = true;
    int samplesPerPixel = juce::jmax(1, numSamples / width);

    for (int x = 0; x < width; ++x)
    {
        int start = (x * numSamples) / width;
        int end   = juce::jmin(start + samplesPerPixel, numSamples);
        float maxSample = 0.0f;

        for (int ch = 0; ch < audioBuffer.getNumChannels(); ++ch)
        {
            const float* data = audioBuffer.getReadPointer(ch);
            for (int s = start; s < end; ++s)
                maxSample = juce::jmax(maxSample, std::abs(data[s]));
        }

        float xPos = static_cast<float>(area.getX() + x);
        float yPos = centerY - (maxSample * height * 0.44f);

        if (first) { waveform.startNewSubPath(xPos, yPos); first = false; }
        else        waveform.lineTo(xPos, yPos);
    }

    // Filled waveform (symmetric)
    juce::Path filled = waveform;
    filled.lineTo(static_cast<float>(area.getRight()), centerY);
    filled.lineTo(static_cast<float>(area.getX()), centerY);
    filled.closeSubPath();

    g.setColour(juce::Colour(PluginColors::accent).withAlpha(0.2f));
    g.fillPath(filled);

    g.setColour(juce::Colour(PluginColors::accent));
    g.strokePath(waveform, juce::PathStrokeType(1.0f));
}

void TriggerUI::TriggerChannel::loadAudioFile(const juce::File& file)
{
    juce::AudioFormatManager fmt;
    fmt.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(fmt.createReaderFor(file));

    if (reader)
    {
        audioBuffer.setSize(static_cast<int>(reader->numChannels),
                            static_cast<int>(reader->lengthInSamples));
        reader->read(&audioBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
        audioSampleRate = reader->sampleRate;
        filenameLabel.setText(file.getFileNameWithoutExtension(), juce::dontSendNotification);
        repaint();
    }
}

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
    scrollView.setScrollBarsShown(true, false);
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
}

void TriggerUI::resized()
{
    scrollView.setBounds(getLocalBounds());

    // Cards: 3 per row, auto height
    int cols   = 3;
    int rows   = (8 + cols - 1) / cols;  // ceil(8/3) = 3
    int margin = 8;
    int cardW  = (getWidth() - margin * (cols + 1)) / cols;
    int cardH  = 280; // fixed card height per requirements

    int totalH = rows * cardH + (rows + 1) * margin;
    channelsContainer.setSize(getWidth(), totalH);

    for (int i = 0; i < 8; ++i)
    {
        int col = i % cols;
        int row = i / cols;
        int cx  = margin + col * (cardW + margin);
        int cy  = margin + row * (cardH + margin);
        channels[i]->setBounds(cx, cy, cardW, cardH);
    }
}

bool TriggerUI::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
        if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".aif") ||
            f.endsWithIgnoreCase(".aiff") || f.endsWithIgnoreCase(".mp3"))
            return true;
    return false;
}

void TriggerUI::fileDragEnter(const juce::StringArray& files, int x, int y)
{
    // Find the channel that the drag is over
    for (auto& ch : channels)
        if (ch && ch->isInterestedInFileDrag(files))
            ch->fileDragEnter(files, x - ch->getX(), y - ch->getY());
}

void TriggerUI::fileDragExit(const juce::StringArray& files)
{
    for (auto& ch : channels)
        if (ch) ch->fileDragExit(files);
}

void TriggerUI::filesDropped(const juce::StringArray& files, int x, int y)
{
    for (auto& ch : channels)
    {
        if (ch && ch->getBounds().contains(x, y))
        {
            ch->filesDropped(files, x - ch->getX(), y - ch->getY());
            return;
        }
    }
}
