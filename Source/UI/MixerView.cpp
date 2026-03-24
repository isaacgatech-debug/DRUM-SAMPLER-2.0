#include "MixerView.h"
#include "../Core/PluginProcessor.h"
#include "../Mixer/MixerChannel.h"

MixerView::MixerView()
{
    createChannelStrips();
    startTimerHz(30); // 30 FPS for meter updates
}

MixerView::~MixerView()
{
    stopTimer();
}

void MixerView::createChannelStrips()
{
    for (int i = 0; i < numChannels; ++i)
    {
        channelStrips[i] = std::make_unique<ChannelStrip>(i, channelNames[i]);
        
        // Set up parameter callbacks
        channelStrips[i]->onParameterChanged = [this](int channel, const juce::String& param, float value)
        {
            DBG("Channel " + juce::String(channel) + " - " + param + ": " + juce::String(value));
            // Route to processor
            if (processor)
            {
                // TODO: Implement parameter routing to processor
            }
        };
        
        channelStrips[i]->onButtonChanged = [this](int channel, const juce::String& btn, bool state)
        {
            DBG("Channel " + juce::String(channel) + " - " + btn + ": " + (state ? "ON" : "OFF"));
            // Route to processor
            if (processor)
            {
                auto* mixerChannel = processor->getMixerChannelForInput(channel);
                if (mixerChannel)
                {
                    if (btn == "Mute")
                        mixerChannel->setMute(state);
                    else if (btn == "Solo")
                        mixerChannel->setSolo(state);
                    else if (btn == "Phase")
                        mixerChannel->setPhaseInvert(state);
                }
            }
        };
        
        channelStrips[i]->onOutputBusChanged = [this](int channel, int busIndex)
        {
            DBG("Channel " + juce::String(channel) + " -> Bus " + juce::String(busIndex));
            if (processor)
            {
                auto* mixerChannel = processor->getMixerChannelForInput(channel);
                if (mixerChannel)
                    mixerChannel->setOutputBus(busIndex);
            }
        };
        
        channelStrips[i]->onInputSourceChanged = [this](int channel, int sourceIndex)
        {
            DBG("Channel " + juce::String(channel) + " Input Source: " + juce::String(sourceIndex));
            if (processor)
            {
                // Route this channel to receive audio from the selected input source
                // This changes which audio stream feeds into this channel
                // TODO: Implement input source routing in processor
            }
        };
        
        addAndMakeVisible(*channelStrips[i]);
    }
}

void MixerView::setProcessor(DrumSampler2Processor* proc)
{
    processor = proc;
    
    for (auto& strip : channelStrips)
    {
        if (strip)
            strip->setProcessor(proc);
    }
}

void MixerView::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(bgColor);
    
    // Draw mixer frame
    drawMixerFrame(g);
}

void MixerView::drawMixerFrame(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Top frame bar
    auto topBar = bounds.removeFromTop(30.0f);
    g.setGradientFill(juce::ColourGradient(
        metalColor.brighter(0.1f), topBar.getX(), topBar.getY(),
        darkMetal, topBar.getX(), topBar.getBottom(),
        false));
    g.fillRect(topBar);
    
    // Title
    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    g.setColour(accentColor);
    g.drawText("MIXER", topBar.reduced(10.0f, 0), juce::Justification::centredLeft, false);
    
    // Right side info
    g.setFont(juce::FontOptions(10.0f));
    g.setColour(juce::Colours::lightgrey.withAlpha(0.7f));
    juce::String infoText = "12 CH | 8 BUSES | OUT: 33";
    g.drawText(infoText, topBar.reduced(10.0f, 0), juce::Justification::centredRight, false);
    
    // Bottom frame bar
    auto bottomBar = bounds.removeFromBottom(20.0f);
    g.setGradientFill(juce::ColourGradient(
        darkMetal, bottomBar.getX(), bottomBar.getY(),
        metalColor.darker(0.2f), bottomBar.getX(), bottomBar.getBottom(),
        false));
    g.fillRect(bottomBar);
    
    // Side rails for punk industrial look
    g.setColour(metalColor.darker(0.3f));
    g.fillRect(bounds.getX(), bounds.getY(), 4.0f, bounds.getHeight());
    g.fillRect(bounds.getRight() - masterWidth - 4.0f, bounds.getY(), 4.0f, bounds.getHeight());
    
    // Draw master section background
    auto masterBounds = bounds.removeFromRight(static_cast<float>(masterWidth));
    drawMasterSection(g, masterBounds);
}

void MixerView::drawMasterSection(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Master section background
    g.setGradientFill(juce::ColourGradient(
        darkMetal.darker(0.1f), bounds.getX(), bounds.getY(),
        darkMetal, bounds.getX(), bounds.getBottom(),
        false));
    g.fillRect(bounds);
    
    // Master label
    auto headerBounds = bounds.removeFromTop(35.0f);
    g.setGradientFill(juce::ColourGradient(
        accentColor.darker(0.5f), headerBounds.getX(), headerBounds.getY(),
        darkMetal.darker(0.2f), headerBounds.getX(), headerBounds.getBottom(),
        false));
    g.fillRect(headerBounds);
    
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("MASTER", headerBounds.reduced(4.0f), juce::Justification::centred, false);
    
    // Master accent line
    g.setColour(accentColor);
    g.fillRect(headerBounds.getX(), headerBounds.getY(), headerBounds.getWidth(), 2.0f);
    
    // Master fader area (placeholder - could add master volume here)
    auto faderBounds = bounds.removeFromTop(120.0f);
    g.setColour(metalColor.withAlpha(0.3f));
    g.fillRect(faderBounds.reduced(8.0f));
    
    g.setFont(juce::FontOptions(9.0f));
    g.setColour(juce::Colours::lightgrey);
    g.drawText("MASTER FADER", faderBounds, juce::Justification::centred, false);
    
    // Output meters area
    auto meterBounds = bounds.removeFromTop(200.0f);
    g.setColour(metalColor.withAlpha(0.3f));
    g.fillRect(meterBounds.reduced(8.0f));
    
    // DAW routing info
    auto routingBounds = bounds.reduced(4.0f);
    g.setFont(juce::FontOptions(8.0f));
    g.setColour(accentColor.withAlpha(0.8f));
    g.drawText("DAW ROUTING", routingBounds.removeFromTop(20.0f), juce::Justification::centred, false);
    
    g.setColour(juce::Colours::lightgrey.withAlpha(0.6f));
    g.drawMultiLineText(
        "All channels routable\nto DAW tracks via\n33 output buses",
        static_cast<int>(routingBounds.getCentreX() - 40),
        static_cast<int>(routingBounds.getY() + 30),
        80);
}

void MixerView::resized()
{
    auto bounds = getLocalBounds();
    
    // Account for frame bars
    bounds.removeFromTop(30);
    bounds.removeFromBottom(20);
    
    // Remove side rails space
    bounds.removeFromLeft(4);
    bounds.removeFromRight(4);
    
    // Master section on right
    auto masterBounds = bounds.removeFromRight(masterWidth);
    
    // Distribute channel strips across remaining space
    int availableWidth = bounds.getWidth();
    int stripCount = numChannels;
    int actualStripWidth = juce::jmin(stripWidth, availableWidth / stripCount);
    
    // If we have more space, center the strips
    int totalStripWidth = actualStripWidth * stripCount;
    int extraSpace = availableWidth - totalStripWidth;
    int leftPadding = extraSpace / 2;
    
    bounds.removeFromLeft(leftPadding);
    
    for (int i = 0; i < numChannels; ++i)
    {
        if (channelStrips[i])
        {
            auto stripBounds = bounds.removeFromLeft(actualStripWidth);
            channelStrips[i]->setBounds(stripBounds);
        }
    }
    
    // Keep master bounds (unused but reserved for future master controls)
    juce::ignoreUnused(masterBounds);
}

void MixerView::timerCallback()
{
    // Update VU meters from processor
    if (processor)
    {
        for (int i = 0; i < numChannels; ++i)
        {
            auto* channel = processor->getMixerChannelForInput(i);
            if (channel && channelStrips[i])
            {
                channelStrips[i]->updateMeter(channel->getPeakLevel(), channel->getRMSLevel());
            }
        }
    }
}
