#include "MixerView.h"
#include "../Core/PluginProcessor.h"
#include "../Mixer/MixerChannel.h"

MixerView::MixerView()
{
    stripsViewport.setViewedComponent(&stripsContainer, false);
    stripsViewport.setScrollBarsShown(false, true);  // vertical off, horizontal on
    stripsViewport.setScrollBarThickness(8);
    addAndMakeVisible(stripsViewport);

    // Master fader
    masterFader.setSliderStyle(juce::Slider::LinearVertical);
    masterFader.setRange(0.0, 1.0, 0.001);
    masterFader.setValue(0.8);
    masterFader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterFader.setColour(juce::Slider::thumbColourId,      juce::Colour(PluginColors::textPrimary));
    masterFader.setColour(juce::Slider::trackColourId,      juce::Colour(PluginColors::accentDim));
    masterFader.setColour(juce::Slider::backgroundColourId, juce::Colour(PluginColors::pluginBg));
    addAndMakeVisible(masterFader);

    masterDbLabel.setText("-2.0", juce::dontSendNotification);
    masterDbLabel.setFont(PluginFonts::mono(10.0f));
    masterDbLabel.setColour(juce::Label::textColourId, juce::Colour(PluginColors::accent));
    masterDbLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(masterDbLabel);

    addAndMakeVisible(masterVU);

    createChannelStrips();
    startTimerHz(30);
}

MixerView::~MixerView()
{
    stopTimer();
}

void MixerView::createChannelStrips()
{
    int totalW = numChannels * ChannelStrip::stripWidth;
    stripsContainer.setSize(totalW, 800);

    for (int i = 0; i < numChannels; ++i)
    {
        channelStrips[i] = std::make_unique<ChannelStrip>(i, channelNames[i]);

        channelStrips[i]->onButtonChanged = [this](int ch, const juce::String& btn, bool state)
        {
            if (processor)
            {
                auto* mc = processor->getMixerChannelForInput(ch);
                if (mc)
                {
                    if (btn == "Mute") mc->setMute(state);
                    else if (btn == "Solo") mc->setSolo(state);
                }
            }
        };

        channelStrips[i]->onParameterChanged = [this](int ch, const juce::String& param, float val)
        {
            juce::ignoreUnused(ch, param, val);
        };

        stripsContainer.addAndMakeVisible(*channelStrips[i]);
    }
}

void MixerView::setProcessor(DrumSampler2Processor* proc)
{
    processor = proc;
    for (auto& strip : channelStrips)
        if (strip) strip->setProcessor(proc);
}

void MixerView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(PluginColors::pluginBg));

    // Sidebar
    auto sidebarBounds = getLocalBounds().toFloat().removeFromLeft(static_cast<float>(sidebarW));
    drawSidebar(g, sidebarBounds);

    // Master separator (2px)
    float masterX = static_cast<float>(getWidth() - masterW);
    g.setColour(juce::Colour(PluginColors::accent).withAlpha(0.3f));
    g.fillRect(masterX - 2.0f, 0.0f, 2.0f, static_cast<float>(getHeight()));
}

void MixerView::drawSidebar(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(juce::Colour(PluginColors::pluginPanel));
    g.fillRect(bounds);

    g.setColour(juce::Colour(PluginColors::pluginBorder));
    g.drawVerticalLine(static_cast<int>(bounds.getRight()) - 1, bounds.getY(), bounds.getBottom());

    // Labels aligned to sections in channel strip layout
    const char* labels[] = { "INPUT", "EQ", "SENDS", "S/M", "VU", "FADER", "PAN" };
    // Approx y positions matching channel strip layout (after 3+36+30=69px header area)
    const int labelYs[] = { 5, 75, 120, 165, 200, 310, 400 };

    g.setFont(PluginFonts::mono(8.0f));
    g.setColour(juce::Colour(PluginColors::textDim));

    for (int i = 0; i < 7; ++i)
    {
        g.drawText(labels[i],
                   static_cast<int>(bounds.getX()), labelYs[i],
                   static_cast<int>(bounds.getWidth()), 12,
                   juce::Justification::centred, false);
        // Separator line
        g.setColour(juce::Colour(PluginColors::pluginBorder).withAlpha(0.4f));
        g.drawHorizontalLine(labelYs[i] - 1, bounds.getX(), bounds.getRight());
        g.setColour(juce::Colour(PluginColors::textDim));
    }
}

void MixerView::drawMasterSection(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(juce::Colour(PluginColors::pluginPanel));
    g.fillRect(bounds);

    // Cyan top bar (3px)
    g.setColour(PluginColors::masterColor);
    g.fillRect(bounds.removeFromTop(3.0f));

    // Header
    auto header = bounds.removeFromTop(36.0f);
    g.setColour(juce::Colour(PluginColors::pluginSurface));
    g.fillRect(header);
    g.setFont(PluginFonts::label(11.0f));
    g.setColour(PluginColors::masterColor);
    g.drawText("MASTER", header, juce::Justification::centred, false);
}

void MixerView::resized()
{
    auto area = getLocalBounds();
    area.removeFromLeft(sidebarW);

    // Master strip (right side)
    auto masterArea = area.removeFromRight(masterW);

    // Viewport fills the rest
    stripsViewport.setBounds(area);

    // Size container to fit all strips at full height
    int containerH = area.getHeight();
    int totalW = numChannels * ChannelStrip::stripWidth;
    stripsContainer.setSize(totalW, containerH);

    // Position each strip
    for (int i = 0; i < numChannels; ++i)
    {
        if (channelStrips[i])
            channelStrips[i]->setBounds(i * ChannelStrip::stripWidth, 0,
                                         ChannelStrip::stripWidth, containerH);
    }

    // Master section components
    int mX = masterArea.getX();
    int mY = masterArea.getY();
    int mH = masterArea.getHeight();
    int mW = masterArea.getWidth();

    // VU left, fader right
    masterVU.setBounds(mX + 4, mY + 45, 20, mH - 100);
    masterFader.setBounds(mX + 30, mY + 45, mW - 34, mH - 100);
    masterDbLabel.setBounds(mX, mY + mH - 48, mW, 14);
}

void MixerView::timerCallback()
{
    if (processor)
    {
        for (int i = 0; i < numChannels; ++i)
        {
            auto* ch = processor->getMixerChannelForInput(i);
            if (ch && channelStrips[i])
                channelStrips[i]->updateMeter(ch->getPeakLevel(), ch->getRMSLevel());
        }
    }
}
