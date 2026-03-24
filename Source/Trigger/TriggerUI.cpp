#include "TriggerUI.h"
#include "DrumClassifier.h"
#include <juce_audio_formats/juce_audio_formats.h>

TriggerUI::TriggerUI()
{
    addAndMakeVisible(loadButton);
    loadButton.onClick = [this] { 
        auto chooser = std::make_shared<juce::FileChooser>("Select audio file", juce::File(), "*.wav;*.aif;*.aiff;*.mp3");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file.existsAsFile())
                    showDrumTypeSelector(file);
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
    
    fileInfoLabel.setText("", juce::dontSendNotification);
    fileInfoLabel.setJustificationType(juce::Justification::centred);
    fileInfoLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(fileInfoLabel);
}

void TriggerUI::paint(juce::Graphics& g)
{
    g.fillAll(bgColour);
    
    auto bounds = getLocalBounds();
    auto waveformArea = bounds.removeFromTop(bounds.getHeight() - 180).reduced(10);
    
    // Draw drag highlight if dragging over
    if (isDragOver)
    {
        g.setColour(dragHighlightColour.withAlpha(0.3f));
        g.fillRect(waveformArea);
        g.setColour(dragHighlightColour);
        g.drawRect(waveformArea, 2);
        g.drawText("Drop audio file here", waveformArea, juce::Justification::centred);
    }
    else if (audioBuffer.getNumSamples() > 0)
    {
        // Draw waveform background
        g.setColour(juce::Colour(0xFF1E1E1E));
        g.fillRect(waveformArea);
        g.setColour(juce::Colours::grey.withAlpha(0.3f));
        g.drawRect(waveformArea);
        
        drawWaveform(g, waveformArea);
        drawTriggerMarkers(g, waveformArea);
    }
    else
    {
        g.setColour(juce::Colours::grey.withAlpha(0.5f));
        g.drawRect(waveformArea, 1);
        g.setColour(juce::Colours::grey);
        g.drawText("Drag & drop .wav, .aif, or .mp3 file here", 
                   waveformArea, juce::Justification::centred);
        g.setFont(12.0f);
        g.drawText("or click Load Audio to browse", 
                   waveformArea.withTrimmedTop(20), juce::Justification::centred);
    }
}

void TriggerUI::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(bounds.getHeight() - 180);
    
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
    
    controlArea.removeFromTop(5);
    fileInfoLabel.setBounds(controlArea.removeFromTop(20));
    
    controlArea.removeFromTop(5);
    statusLabel.setBounds(controlArea.removeFromTop(30));
}

bool TriggerUI::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& file : files)
    {
        if (file.endsWithIgnoreCase(".wav") || 
            file.endsWithIgnoreCase(".aif") ||
            file.endsWithIgnoreCase(".aiff") ||
            file.endsWithIgnoreCase(".mp3"))
            return true;
    }
    return false;
}

void TriggerUI::fileDragEnter(const juce::StringArray& files, int, int)
{
    if (isInterestedInFileDrag(files))
    {
        isDragOver = true;
        repaint();
    }
}

void TriggerUI::fileDragExit(const juce::StringArray&)
{
    isDragOver = false;
    repaint();
}

void TriggerUI::filesDropped(const juce::StringArray& files, int, int)
{
    isDragOver = false;
    if (!files.isEmpty())
    {
        showDrumTypeSelector(juce::File(files[0]));
    }
}

void TriggerUI::showDrumTypeSelector(const juce::File& file)
{
    auto* alert = new juce::AlertWindow("Select Drum Type", 
                                        "Which drum type would you like to detect in: " + file.getFileName() + "?",
                                        juce::AlertWindow::QuestionIcon);
    
    alert->addButton("Kick (Bass Drum)", 1);
    alert->addButton("Snare", 2);
    alert->addButton("Hi-Hat", 3);
    alert->addButton("Tom", 4);
    alert->addButton("Crash", 5);
    alert->addButton("Ride", 6);
    alert->addButton("All Drums (Auto)", 7);
    alert->addButton("Cancel", 0);
    
    alert->enterModalState(true, juce::ModalCallbackFunction::create([this, file](int result) {
        if (result == 0) return; // Cancelled
        
        switch (result)
        {
            case 1: targetDrumType = DrumType::Kick; break;
            case 2: targetDrumType = DrumType::Snare; break;
            case 3: targetDrumType = DrumType::HiHat; break;
            case 4: targetDrumType = DrumType::Tom; break;
            case 5: targetDrumType = DrumType::Crash; break;
            case 6: targetDrumType = DrumType::Ride; break;
            case 7: targetDrumType = DrumType::Unknown; break; // Auto detect all
            default: targetDrumType = DrumType::Unknown; break;
        }
        
        if (triggerEngine)
            triggerEngine->setTargetDrumType(targetDrumType);
        
        loadAudioFile(file);
    }), true);
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
        audioSampleRate = reader->sampleRate;
        
        processButton.setEnabled(true);
        
        DrumClassifier classifier;
        juce::String drumTypeName;
        switch (targetDrumType)
        {
            case DrumType::Kick: drumTypeName = "Kick"; break;
            case DrumType::Snare: drumTypeName = "Snare"; break;
            case DrumType::HiHat: drumTypeName = "Hi-Hat"; break;
            case DrumType::Tom: drumTypeName = "Tom"; break;
            case DrumType::Crash: drumTypeName = "Crash"; break;
            case DrumType::Ride: drumTypeName = "Ride"; break;
            default: drumTypeName = "All drums (auto-detect)"; break;
        }
        
        statusLabel.setText("Loaded: " + file.getFileName(), juce::dontSendNotification);
        fileInfoLabel.setText("Target: " + drumTypeName + " | " + 
                              formatDuration(audioBuffer.getNumSamples(), audioSampleRate) + " | " +
                              juce::String(audioSampleRate, 0) + " Hz | " +
                              juce::String(audioBuffer.getNumChannels()) + " ch",
                              juce::dontSendNotification);
        repaint();
    }
    else
    {
        statusLabel.setText("Error: Could not load " + file.getFileName(), juce::dontSendNotification);
        fileInfoLabel.setText("", juce::dontSendNotification);
    }
}

juce::String TriggerUI::formatDuration(double samples, double sampleRate)
{
    double seconds = samples / sampleRate;
    int mins = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    int ms = static_cast<int>((seconds - static_cast<int>(seconds)) * 1000);
    return juce::String::formatted("%02d:%02d.%03d", mins, secs, ms);
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
                                 audioSampleRate, audioBuffer.getNumChannels(), 16, {}, 0));
    
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
    if (audioBuffer.getNumSamples() == 0 || area.getWidth() <= 0)
        return;
    
    const int numSamples = audioBuffer.getNumSamples();
    const int width = area.getWidth();
    const float height = static_cast<float>(area.getHeight());
    const float centerY = area.getY() + height / 2.0f;
    
    // Draw center line
    g.setColour(juce::Colours::grey.withAlpha(0.3f));
    g.drawHorizontalLine(static_cast<int>(centerY), static_cast<float>(area.getX()), static_cast<float>(area.getRight()));
    
    // Draw waveform with proper scaling
    g.setColour(waveformColour);
    
    juce::Path minPath, maxPath;
    bool first = true;
    
    // Calculate samples per pixel for efficient drawing
    const int samplesPerPixel = juce::jmax(1, numSamples / width);
    
    for (int x = 0; x < width; ++x)
    {
        int startSample = (x * numSamples) / width;
        int endSample = juce::jmin(startSample + samplesPerPixel, numSamples);
        
        float minSample = 0.0f;
        float maxSample = 0.0f;
        
        // Find min/max in this pixel's range
        for (int ch = 0; ch < audioBuffer.getNumChannels(); ++ch)
        {
            const float* channelData = audioBuffer.getReadPointer(ch);
            for (int s = startSample; s < endSample; ++s)
            {
                float sample = channelData[s];
                minSample = juce::jmin(minSample, sample);
                maxSample = juce::jmax(maxSample, sample);
            }
        }
        
        float xPos = static_cast<float>(area.getX() + x);
        float minY = centerY - (minSample * height * 0.45f);
        float maxY = centerY - (maxSample * height * 0.45f);
        
        if (first)
        {
            minPath.startNewSubPath(xPos, minY);
            maxPath.startNewSubPath(xPos, maxY);
            first = false;
        }
        else
        {
            minPath.lineTo(xPos, minY);
            maxPath.lineTo(xPos, maxY);
        }
    }
    
    // Create filled waveform
    juce::Path waveformPath = minPath;
    waveformPath.lineTo(static_cast<float>(area.getRight()), centerY);
    waveformPath.lineTo(static_cast<float>(area.getX()), centerY);
    waveformPath.closeSubPath();
    
    g.setColour(waveformColour.withAlpha(0.3f));
    g.fillPath(waveformPath);
    
    g.setColour(waveformColour);
    g.strokePath(minPath, juce::PathStrokeType(1.0f));
    g.strokePath(maxPath, juce::PathStrokeType(1.0f));
}

void TriggerUI::drawTriggerMarkers(juce::Graphics& g, const juce::Rectangle<int>& area)
{
    if (audioBuffer.getNumSamples() == 0)
        return;
    
    g.setColour(triggerColour);
    
    for (const auto& trigger : triggerResults)
    {
        float x = area.getX() + (static_cast<float>(trigger.timeInSamples) / audioBuffer.getNumSamples()) * area.getWidth();
        
        // Draw vertical trigger line
        g.drawVerticalLine(static_cast<int>(x), static_cast<float>(area.getY()), 
                          static_cast<float>(area.getBottom()));
        
        // Draw drum type label above the line
        juce::String label;
        switch (trigger.drumType)
        {
            case DrumType::Kick: label = "K"; break;
            case DrumType::Snare: label = "S"; break;
            case DrumType::HiHat: label = "H"; break;
            case DrumType::Tom: label = "T"; break;
            case DrumType::Crash: label = "C"; break;
            case DrumType::Ride: label = "R"; break;
            default: label = juce::String(trigger.midiNote); break;
        }
        
        g.setColour(triggerColour.withAlpha(0.8f));
        g.fillEllipse(x - 8, static_cast<float>(area.getY()) + 5, 16, 16);
        g.setColour(juce::Colours::black);
        g.drawText(label, static_cast<int>(x) - 10, area.getY() + 5, 20, 16,
                  juce::Justification::centred);
        g.setColour(triggerColour);
    }
}
