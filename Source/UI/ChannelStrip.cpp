#include "ChannelStrip.h"
#include "../Core/PluginProcessor.h"
#include "../Mixer/MixerChannel.h"

ChannelStrip::ChannelStrip(int channelIndex, const juce::String& channelName)
    : index(channelIndex), name(channelName)
{
    // Assign channel-specific accent color
    juce::uint8 hue = (channelIndex * 25) % 360;
    channelColor = juce::Colour::fromHSV(hue / 360.0f, 0.8f, 0.9f, 1.0f);
    
    setupKnobs();
    setupRouting();
    
    // Setup buttons with punk styling
    auto setupPunkButton = [this](juce::TextButton& btn, juce::Colour activeColor)
    {
        btn.setColour(juce::TextButton::buttonColourId, darkMetal);
        btn.setColour(juce::TextButton::buttonOnColourId, activeColor);
        btn.setColour(juce::TextButton::textColourOffId, juce::Colours::lightgrey);
        btn.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        btn.setClickingTogglesState(true);
        btn.addListener(this);
        addAndMakeVisible(btn);
    };
    
    setupPunkButton(muteButton, punkRed.darker(0.3f));
    setupPunkButton(soloButton, punkGreen.darker(0.3f));
    setupPunkButton(phaseButton, accentColor.darker(0.3f));
}

void ChannelStrip::setupKnobs()
{
    auto setupKnob = [this](AnalogKnob& knob, const juce::String& label, float min, float max, float defaultVal)
    {
        knob.setLabel(label);
        knob.setRange(min, max);
        knob.setValue(defaultVal);
        knob.setKnobColor(channelColor);
        knob.onValueChange = [this, &knob, label]
        {
            if (onParameterChanged)
                onParameterChanged(index, label, static_cast<float>(knob.getValue()));
        };
        addAndMakeVisible(knob);
    };
    
    // EQ knobs
    setupKnob(eqLowFreq, "Low Freq", 20.0f, 500.0f, 100.0f);
    setupKnob(eqLowGain, "Low Gain", -15.0f, 15.0f, 0.0f);
    setupKnob(eqMidFreq, "Mid Freq", 200.0f, 5000.0f, 1000.0f);
    setupKnob(eqMidGain, "Mid Gain", -15.0f, 15.0f, 0.0f);
    setupKnob(eqHighFreq, "High Freq", 2000.0f, 20000.0f, 10000.0f);
    setupKnob(eqHighGain, "High Gain", -15.0f, 15.0f, 0.0f);
    
    // Compressor knobs
    setupKnob(compThreshold, "Threshold", -60.0f, 0.0f, -20.0f);
    setupKnob(compRatio, "Ratio", 1.0f, 20.0f, 4.0f);
    setupKnob(compAttack, "Attack", 0.1f, 100.0f, 10.0f);
    setupKnob(compRelease, "Release", 10.0f, 1000.0f, 100.0f);
    setupKnob(compMakeup, "Makeup", 0.0f, 20.0f, 0.0f);
    
    // Channel controls
    setupKnob(gainKnob, "Gain", 0.0f, 2.0f, 1.0f);
    gainKnob.setKnobColor(accentColor);
    
    setupKnob(panKnob, "Pan", -1.0f, 1.0f, 0.0f);
}

void ChannelStrip::setupRouting()
{
    // DAW Input selector - physical audio inputs from the interface
    // This allows routing external audio (e.g., mic preamps) into the channel
    dawInputSelector.addItem("Internal", 1);  // Default - uses internal sampler
    dawInputSelector.addItem("Input 1 (L)", 2);
    dawInputSelector.addItem("Input 2 (R)", 3);
    dawInputSelector.addItem("Input 3", 4);
    dawInputSelector.addItem("Input 4", 5);
    dawInputSelector.addItem("Input 5", 6);
    dawInputSelector.addItem("Input 6", 7);
    dawInputSelector.addItem("Input 7", 8);
    dawInputSelector.addItem("Input 8", 9);
    dawInputSelector.setSelectedId(1, juce::dontSendNotification);
    dawInputSelector.setColour(juce::ComboBox::backgroundColourId, darkMetal);
    dawInputSelector.setColour(juce::ComboBox::textColourId, juce::Colours::lightgrey);
    dawInputSelector.setColour(juce::ComboBox::outlineColourId, metalColor);
    dawInputSelector.onChange = [this]
    {
        if (onInputSourceChanged)
            onInputSourceChanged(index, dawInputSelector.getSelectedId() - 1);
    };
    addAndMakeVisible(dawInputSelector);
    
    // Input source selector - lists all available drum inputs
    const juce::StringArray inputSources = {
        "Kick In", "Kick Out", "Snare Top", "Snare Bottom",
        "Tom 1", "Tom 2", "Tom 3", "OVH L", "OVH R",
        "Hat", "RM L", "RM R"
    };
    
    inputSelector.addItemList(inputSources, 1);
    inputSelector.setSelectedId(index + 1, juce::dontSendNotification); // Default to matching channel
    inputSelector.setColour(juce::ComboBox::backgroundColourId, darkMetal);
    inputSelector.setColour(juce::ComboBox::textColourId, juce::Colours::lightgrey);
    inputSelector.setColour(juce::ComboBox::outlineColourId, metalColor);
    inputSelector.onChange = [this]
    {
        if (onInputSourceChanged)
            onInputSourceChanged(index, inputSelector.getSelectedId() - 1);
    };
    addAndMakeVisible(inputSelector);
    
    // Output selector
    outputSelector.addItemList({"Master", "Bus 1", "Bus 2", "Bus 3", "Bus 4", 
                                "Bus 5", "Bus 6", "Bus 7", "Bus 8"}, 1);
    outputSelector.setSelectedId(1, juce::dontSendNotification);
    outputSelector.setColour(juce::ComboBox::backgroundColourId, darkMetal);
    outputSelector.setColour(juce::ComboBox::textColourId, juce::Colours::lightgrey);
    outputSelector.setColour(juce::ComboBox::outlineColourId, metalColor);
    outputSelector.onChange = [this]
    {
        if (onOutputBusChanged)
            onOutputBusChanged(index, outputSelector.getSelectedId() - 1);
    };
    addAndMakeVisible(outputSelector);
    
    // Meter
    addAndMakeVisible(vuMeter);
}

void ChannelStrip::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background - distressed metal strip
    g.setGradientFill(juce::ColourGradient(
        darkMetal.brighter(0.05f), bounds.getX(), bounds.getY(),
        darkMetal, bounds.getX(), bounds.getBottom(),
        false));
    g.fillRect(bounds);
    
    // Side borders for strip separation
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawVerticalLine(static_cast<int>(bounds.getX()), bounds.getY(), bounds.getBottom());
    g.drawVerticalLine(static_cast<int>(bounds.getRight()) - 1, bounds.getY(), bounds.getBottom());
    
    // Channel header with name
    auto headerBounds = bounds.removeFromTop(35.0f);
    drawChannelHeader(g, headerBounds);
    
    // Draw section dividers
    float sectionHeight = bounds.getHeight() / 4.5f;
    
    // EQ Section
    auto eqBounds = bounds.removeFromTop(sectionHeight);
    drawSectionDivider(g, eqBounds.removeFromTop(16.0f), "EQ");
    
    // Compressor Section
    auto compBounds = bounds.removeFromTop(sectionHeight);
    drawSectionDivider(g, compBounds.removeFromTop(16.0f), "COMP");
    
    // Routing Section
    auto routingBounds = bounds.removeFromTop(sectionHeight * 0.9f);
    drawSectionDivider(g, routingBounds.removeFromTop(16.0f), "I/O");
}

void ChannelStrip::drawChannelHeader(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Header background with channel color accent
    g.setGradientFill(juce::ColourGradient(
        channelColor.darker(0.4f), bounds.getX(), bounds.getY(),
        darkMetal.darker(0.2f), bounds.getX(), bounds.getBottom(),
        false));
    g.fillRect(bounds);
    
    // Top accent line
    g.setColour(channelColor);
    g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), 2.0f);
    
    // Channel number
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.drawText(juce::String(index + 1), bounds.removeFromTop(14.0f).reduced(2.0f), 
               juce::Justification::topLeft, false);
    
    // Channel name
    g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    
    // Truncate if needed
    juce::String displayName = name;
    if (displayName.length() > 10)
        displayName = displayName.substring(0, 8) + "..";
    
    g.drawText(displayName, bounds.reduced(2.0f), juce::Justification::centred, false);
}

void ChannelStrip::drawSectionDivider(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& label)
{
    // Divider background
    g.setColour(metalColor.withAlpha(0.5f));
    g.fillRect(bounds);
    
    // Label
    g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    g.setColour(accentColor);
    g.drawText(label, bounds, juce::Justification::centred, false);
    
    // Top and bottom lines
    g.setColour(metalColor.darker(0.3f));
    g.drawHorizontalLine(static_cast<int>(bounds.getY()), bounds.getX(), bounds.getRight());
    g.drawHorizontalLine(static_cast<int>(bounds.getBottom()) - 1, bounds.getX(), bounds.getRight());
}

void ChannelStrip::resized()
{
    auto bounds = getLocalBounds();
    
    // Header is painted, not a component
    bounds.removeFromTop(35);
    
    // EQ Section (2 rows x 3 knobs)
    auto eqSection = bounds.removeFromTop(95);
    eqSection.removeFromTop(16); // Label area
    
    auto eqTopRow = eqSection.removeFromTop(38);
    eqLowFreq.setBounds(eqTopRow.removeFromLeft(stripWidth / 2).reduced(2));
    eqLowGain.setBounds(eqTopRow.reduced(2));
    
    auto eqBottomRow = eqSection.reduced(2);
    eqMidFreq.setBounds(eqBottomRow.removeFromLeft(stripWidth / 2).reduced(2));
    eqMidGain.setBounds(eqBottomRow.reduced(2));
    
    // High EQ on separate row for better layout
    // (We'll put it in compressor section since we have limited space)
    
    // Compressor Section (compact layout)
    auto compSection = bounds.removeFromTop(80);
    compSection.removeFromTop(16); // Label area
    
    auto compRow1 = compSection.removeFromTop(38);
    compThreshold.setBounds(compRow1.removeFromLeft(stripWidth / 2).reduced(2));
    compRatio.setBounds(compRow1.reduced(2));
    
    auto compRow2 = compSection.reduced(2);
    compAttack.setBounds(compRow2.removeFromLeft(stripWidth / 2).reduced(2));
    compRelease.setBounds(compRow2.reduced(2));
    
    // Routing Section - now includes DAW input, internal source, and output
    auto routingSection = bounds.removeFromTop(75);
    routingSection.removeFromTop(16); // Label area
    
    // DAW Input selector (top) - for external audio interface inputs
    auto dawInputRow = routingSection.removeFromTop(20);
    dawInputSelector.setBounds(dawInputRow.reduced(2));
    
    // Internal Input source selector (middle) - for internal drum samples
    auto inputRow = routingSection.removeFromTop(20);
    inputSelector.setBounds(inputRow.reduced(2));
    
    // Output selector (bottom) - where this channel sends to
    auto outputRow = routingSection.reduced(2);
    outputSelector.setBounds(outputRow.reduced(2));
    
    // Buttons row
    auto buttonSection = bounds.removeFromTop(35);
    auto btnWidth = stripWidth / 3;
    muteButton.setBounds(buttonSection.removeFromLeft(btnWidth).reduced(2));
    soloButton.setBounds(buttonSection.removeFromLeft(btnWidth).reduced(2));
    phaseButton.setBounds(buttonSection.reduced(2));
    
    // Pan knob
    auto panSection = bounds.removeFromTop(45);
    panKnob.setBounds(panSection.reduced(8, 2));
    
    // VU Meter
    auto meterSection = bounds.removeFromTop(100);
    vuMeter.setBounds(meterSection.reduced(20, 4));
    
    // Main Gain (fader-style at bottom)
    auto gainSection = bounds.removeFromTop(80);
    gainKnob.setBounds(gainSection.reduced(8, 4));
}

void ChannelStrip::setProcessor(DrumSampler2Processor* proc)
{
    processor = proc;
}

void ChannelStrip::updateMeter(float level, float peak)
{
    vuMeter.setLevel(level);
    vuMeter.setPeak(peak);
}

void ChannelStrip::buttonClicked(juce::Button* button)
{
    if (button == &muteButton)
    {
        if (onButtonChanged)
            onButtonChanged(index, "Mute", muteButton.getToggleState());
    }
    else if (button == &soloButton)
    {
        if (onButtonChanged)
            onButtonChanged(index, "Solo", soloButton.getToggleState());
    }
    else if (button == &phaseButton)
    {
        if (onButtonChanged)
            onButtonChanged(index, "Phase", phaseButton.getToggleState());
    }
}
