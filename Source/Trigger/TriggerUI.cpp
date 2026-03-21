#include "TriggerUI.h"
#include <juce_audio_formats/juce_audio_formats.h>

TriggerUI::TriggerUI()
{
    addAndMakeVisible(loadButton);
    loadButton.onClick = [this] { 
        auto chooser = std::make_shared<juce::FileChooser>("Select audio file", juce::File(), "*.wav;*.aif;*.mp3");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file.existsAsFile())
                    loadAudioFile(file);
            });
    };
    
    addAndMakeVisible(processButton);
    processButton.onClick = [this] { processCurrentFile(); };
    processButton.setEnabled(false);
    
    addAndMakeVisible(exportButton);
    exportButton.onClick = [this] {
        auto chooser = std::make_shared<juce::FileChooser>("Export MIDI", juce::File(), "*.mid");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file != juce::File() && triggerEngine)
                    triggerEngine->exportToMIDI(triggerResults, file);
            });
    };
    exportButton.setEnabled(false);
    
    thresholdSlider.setRange(0.0, 1.0, 0.01);
    thresholdSlider.setValue(0.3);
    thresholdSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    thresholdSlider.onValueChange = [this] { 
        if (triggerEngine)
            triggerEngine->setThreshold(static_cast<float>(thresholdSlider.getValue()));
    };
    addAndMakeVisible(thresholdSlider);
    
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    addAndMakeVisible(thresholdLabel);
    
    bleedSlider.setRange(0.0, 1.0, 0.01);
    bleedSlider.setValue(0.5);
    bleedSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    bleedSlider.onValueChange = [this] {
        if (triggerEngine)
            triggerEngine->setBleedSuppression(static_cast<float>(bleedSlider.getValue()));
    };
    addAndMakeVisible(bleedSlider);
    
    bleedLabel.setText("Bleed Suppression", juce::dontSendNotification);
    addAndMakeVisible(bleedLabel);
    
    statusLabel.setText("Drop audio file or click Load Audio", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);
}

void TriggerUI::paint(juce::Graphics& g)
{
    g.fillAll(bgColour);
    
    auto bounds = getLocalBounds();
    auto waveformArea = bounds.removeFromTop(bounds.getHeight() - 150).reduced(10);
    
    if (audioBuffer.getNumSamples() > 0)
    {
        drawWaveform(g, waveformArea);
        drawTriggerMarkers(g, waveformArea);
    }
    else
    {
        g.setColour(juce::Colours::grey);
        g.drawRect(waveformArea);
        g.drawText("No audio loaded", waveformArea, juce::Justification::centred);
    }
}

void TriggerUI::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(bounds.getHeight() - 150);
    
    auto controlArea = bounds.reduced(10);
    
    auto buttonArea = controlArea.removeFromTop(40);
    loadButton.setBounds(buttonArea.removeFromLeft(120).reduced(2));
    processButton.setBounds(buttonArea.removeFromLeft(120).reduced(2));
    exportButton.setBounds(buttonArea.removeFromLeft(120).reduced(2));
    
    controlArea.removeFromTop(10);
    
    auto thresholdArea = controlArea.removeFromTop(30);
    thresholdLabel.setBounds(thresholdArea.removeFromLeft(150));
    thresholdSlider.setBounds(thresholdArea);
    
    controlArea.removeFromTop(5);
    
    auto bleedArea = controlArea.removeFromTop(30);
    bleedLabel.setBounds(bleedArea.removeFromLeft(150));
    bleedSlider.setBounds(bleedArea);
    
    controlArea.removeFromTop(10);
    statusLabel.setBounds(controlArea.removeFromTop(30));
}

bool TriggerUI::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& file : files)
    {
        if (file.endsWithIgnoreCase(".wav") || 
            file.endsWithIgnoreCase(".aif") ||
            file.endsWithIgnoreCase(".mp3"))
            return true;
    }
    return false;
}

void TriggerUI::filesDropped(const juce::StringArray& files, int, int)
{
    if (!files.isEmpty())
    {
        loadAudioFile(juce::File(files[0]));
    }
}

void TriggerUI::loadAudioFile(const juce::File& file)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    
    if (reader != nullptr)
    {
        audioBuffer.setSize(static_cast<int>(reader->numChannels),
                           static_cast<int>(reader->lengthInSamples));
        reader->read(&audioBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
        
        processButton.setEnabled(true);
        statusLabel.setText("Audio loaded: " + file.getFileName(), juce::dontSendNotification);
        repaint();
    }
}

void TriggerUI::processCurrentFile()
{
    if (triggerEngine == nullptr || audioBuffer.getNumSamples() == 0)
        return;
    
    triggerResults.clear();
    
    juce::File tempFile = juce::File::createTempFile(".wav");
    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(new juce::FileOutputStream(tempFile),
                                 44100.0, audioBuffer.getNumChannels(), 16, {}, 0));
    
    if (writer != nullptr)
    {
        writer->writeFromAudioSampleBuffer(audioBuffer, 0, audioBuffer.getNumSamples());
        writer.reset();
        
        triggerEngine->processAudioFile(tempFile, triggerResults);
        tempFile.deleteFile();
        
        exportButton.setEnabled(!triggerResults.empty());
        statusLabel.setText(juce::String(triggerResults.size()) + " triggers detected", 
                          juce::dontSendNotification);
        repaint();
    }
}

void TriggerUI::setThreshold(float threshold)
{
    thresholdSlider.setValue(threshold, juce::dontSendNotification);
}

void TriggerUI::setBleedSuppression(float amount)
{
    bleedSlider.setValue(amount, juce::dontSendNotification);
}

void TriggerUI::drawWaveform(juce::Graphics& g, const juce::Rectangle<int>& area)
{
    g.setColour(waveformColour);
    
    const int numSamples = audioBuffer.getNumSamples();
    const int width = area.getWidth();
    const float height = static_cast<float>(area.getHeight());
    const float centerY = area.getY() + height / 2.0f;
    
    juce::Path waveformPath;
    waveformPath.startNewSubPath(static_cast<float>(area.getX()), centerY);
    
    for (int x = 0; x < width; ++x)
    {
        int sampleIndex = (x * numSamples) / width;
        float sample = audioBuffer.getSample(0, sampleIndex);
        float y = centerY - (sample * height * 0.4f);
        waveformPath.lineTo(static_cast<float>(area.getX() + x), y);
    }
    
    g.strokePath(waveformPath, juce::PathStrokeType(1.0f));
}

void TriggerUI::drawTriggerMarkers(juce::Graphics& g, const juce::Rectangle<int>& area)
{
    if (audioBuffer.getNumSamples() == 0)
        return;
    
    g.setColour(triggerColour);
    
    for (const auto& trigger : triggerResults)
    {
        float x = area.getX() + (static_cast<float>(trigger.timeInSamples) / audioBuffer.getNumSamples()) * area.getWidth();
        g.drawVerticalLine(static_cast<int>(x), static_cast<float>(area.getY()), 
                          static_cast<float>(area.getBottom()));
        
        g.drawText(juce::String(trigger.midiNote), 
                  static_cast<int>(x) - 10, area.getY(), 20, 20,
                  juce::Justification::centred);
    }
}
