#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MicBusLayout.h"
#include "../Effects/ParametricEQEffect.h"
#include <string>

DrumTechProcessor::DrumTechProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "DrumTechState", createParameterLayout())
{
    // One mixer strip per mic stem (not per MIDI note)
    for (int i = 0; i < MicBus::count; ++i)
    {
        mixerChannels.push_back(
            std::make_unique<MixerChannel>(0, juce::String(MicBus::rawName(i)))
        );
    }

    for (auto& noteFlag : recentMidiNotes)
        noteFlag.store(false, std::memory_order_relaxed);
    
    // Initialize with safe defaults so clicks work before host calls prepareToPlay
    samplerEngine.prepareToPlay(44100.0, 512);
    // MIDI-first v1: keep trigger engine compiled, but disable runtime trigger path.
    triggerEngine.enableRealTimeMode(false);
    grooveLibrary.scanFolders();
    presetManager.scanPresets();
}

DrumTechProcessor::~DrumTechProcessor()
{
}

void DrumTechProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    samplerEngine.prepareToPlay(sampleRate, samplesPerBlock);
    busManager.prepare(sampleRate, samplesPerBlock);

    if (!builtinEqInserted)
    {
        for (int i = 0; i < static_cast<int>(mixerChannels.size()); ++i)
        {
            mixerChannels[static_cast<size_t>(i)]->loadPlugin(
                0, std::make_unique<ParametricEQEffect>(*this, i));
        }
        builtinEqInserted = true;
    }

    for (auto& channel : mixerChannels)
    {
        channel->prepare(sampleRate, samplesPerBlock);
    }
    for (auto& stemBuffer : mixerStemBuffers)
        stemBuffer.setSize(2, samplesPerBlock, false, false, true);

    midiPlayer.prepare(sampleRate);
    triggerEngine.prepare(sampleRate, samplesPerBlock);

    applyMixerParameters();
}

void DrumTechProcessor::releaseResources()
{
}

void DrumTechProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    if (triggerEngine.isRealTimeMode() && getTotalNumInputChannels() > 0)
        triggerEngine.processBlock(buffer, midiMessages);

    buffer.clear();

    const int uiMidiMerged = drainUiMidiToBuffer(midiMessages, buffer.getNumSamples());
    juce::ignoreUnused(uiMidiMerged);

    midiPlayer.processBlock(midiMessages, buffer.getNumSamples());
    
    // Check incoming MIDI and set flags for UI
    if (!midiMessages.isEmpty())
    {
        // Set atomic flags for MIDI notes (thread-safe)
        for (const auto metadata : midiMessages)
        {
            auto message = metadata.getMessage();
            if (message.isNoteOn())
            {
                int note = message.getNoteNumber();
                if (note >= 0 && note < 128)
                {
                    recentMidiNotes[note].store(true);
                }
            }
        }
    }

    busManager.clearAllBuses();

    if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("samplerDrummerProfile")))
    {
        switch (p->getIndex())
        {
            case 1: samplerEngine.setDrummerProfile("drummera"); break;
            case 2: samplerEngine.setDrummerProfile("drummerb"); break;
            default: samplerEngine.setDrummerProfile("default"); break;
        }
    }
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("samplerPlayingStyle")))
    {
        switch (p->getIndex())
        {
            case 1: samplerEngine.setPlayingStyle("sticks"); break;
            case 2: samplerEngine.setPlayingStyle("brushes"); break;
            default: samplerEngine.setPlayingStyle("auto"); break;
        }
    }

    samplerEngine.processBlock(buffer, midiMessages);

    for (int channelIndex = 0; channelIndex < static_cast<int>(mixerChannels.size()); ++channelIndex)
    {
        auto* mixerChannel = getMixerChannelForInput(channelIndex);
        if (mixerChannel == nullptr)
            continue;

        auto& stemBuffer = mixerStemBuffers[static_cast<size_t>(channelIndex)];
        jassert(channelIndex >= 0 && channelIndex < SamplerEngine::NUM_DRUM_CHANNELS);
        const auto& sourceStem = samplerEngine.getChannelBuffer(channelIndex);
        stemBuffer.makeCopyOf(sourceStem, true);
        mixerChannel->processAudio(stemBuffer);

        auto& outputBus = busManager.getBus(channelIndex + 1);
        outputBus.getBuffer().makeCopyOf(stemBuffer, true);
    }
    busManager.processAllBuses(buffer);

    if (auto* masterLevel = apvts.getRawParameterValue("masterLevel"))
        buffer.applyGain(masterLevel->load());
    
    // TEMPORARILY DISABLED - mixer channels need per-note routing, not whole-buffer processing
    // for (auto& channel : mixerChannels)
    // {
    //     DBG("*** Calling channel->processAudio ***");
    //     channel->processAudio(buffer);
    //     DBG("*** channel->processAudio returned ***");
    // }
    
    // DBG("*** Calling busManager.processAllBuses ***");
    // busManager.processAllBuses(buffer);
    // DBG("*** busManager.processAllBuses returned ***");
    
    // Keepalive: output an inaudible ~-200dB signal so Logic never suspends
    // processBlock when transport is stopped and no MIDI keyboard is active.
    // Without this, click-triggered notes queue up but never render.
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.addSample(ch, 0, 1.0e-10f);

    applyMixerParameters();
}

juce::AudioProcessorEditor* DrumTechProcessor::createEditor()
{
    return new DrumTechEditor(*this);
}

void DrumTechProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty(StateManager::kSchemaVersionKey,
                      StateManager::kCurrentSchemaVersion, nullptr);
    state.setProperty("legacySampleFolder", lastLoadedFolder.getFullPathName(), nullptr);
    samplerEngine.serializeMicTrims(state);
    StateManager::saveState(state, destData);
}

void DrumTechProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto loadedState = StateManager::loadState(data, sizeInBytes);
    if (loadedState.isValid())
    {
        loadedState = StateManager::migrateStateIfNeeded(loadedState);
        apvts.replaceState(loadedState);
        samplerEngine.deserializeMicTrims(loadedState);

        const auto folderPath = loadedState.getProperty("legacySampleFolder").toString();
        if (folderPath.isNotEmpty())
        {
            juce::File folder(folderPath);
            if (folder.exists())
                loadSamplesFromFolder(folder);
        }
        return;
    }

    // Backward compatibility: very old state stored only a raw folder path string.
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    const juce::String legacyFolderPath = stream.readString();
    if (legacyFolderPath.isNotEmpty())
    {
        juce::File folder(legacyFolderPath);
        if (folder.exists())
            loadSamplesFromFolder(folder);
    }
}

void DrumTechProcessor::loadSamplesFromFolder(const juce::File& folder)
{
    lastLoadedFolder = folder;
    samplerEngine.loadSamplesFromFolder(folder);
}

MixerChannel& DrumTechProcessor::getMixerChannel(int index)
{
    return *mixerChannels[juce::jlimit(0, static_cast<int>(mixerChannels.size()) - 1, index)];
}

MixerChannel* DrumTechProcessor::getMixerChannelForInput(int inputIndex)
{
    if (inputIndex >= 0 && inputIndex < static_cast<int>(mixerChannels.size()))
        return mixerChannels[inputIndex].get();
    return nullptr;
}

void DrumTechProcessor::triggerNote(int midiNote, int velocity)
{
    enqueueUiMidiNote(midiNote, velocity);
    
    if (midiNote >= 0 && midiNote < 128)
        recentMidiNotes[midiNote].store(true);
}

void DrumTechProcessor::setTransportPlaying(bool shouldPlay)
{
    if (auto* play = apvts.getRawParameterValue("transportPlaying"))
        play->store(shouldPlay ? 1.0f : 0.0f);

    if (shouldPlay)
        midiPlayer.play();
    else
        midiPlayer.stop();
}

void DrumTechProcessor::setTransportLooping(bool shouldLoop)
{
    if (auto* loop = apvts.getRawParameterValue("transportLoop"))
        loop->store(shouldLoop ? 1.0f : 0.0f);
    midiPlayer.setLoop(shouldLoop);
}

void DrumTechProcessor::setTransportRecording(bool shouldRecord)
{
    if (auto* record = apvts.getRawParameterValue("transportRecord"))
        record->store(shouldRecord ? 1.0f : 0.0f);
}

void DrumTechProcessor::setTransportTempo(float bpm)
{
    const float clampedBpm = juce::jlimit(30.0f, 300.0f, bpm);
    if (auto* tempo = apvts.getRawParameterValue("transportTempo"))
        tempo->store(clampedBpm);
    midiPlayer.setTempo(clampedBpm);
}

void DrumTechProcessor::setTriggerThreshold(float threshold)
{
    const float clampedThreshold = juce::jlimit(0.0f, 1.0f, threshold);
    if (auto* triggerThreshold = apvts.getRawParameterValue("triggerGlobalThreshold"))
        triggerThreshold->store(clampedThreshold);
    triggerEngine.setThreshold(clampedThreshold);
}

bool DrumTechProcessor::saveCurrentPreset(const juce::String& name, const juce::String& category)
{
    auto state = apvts.copyState();
    state.setProperty(StateManager::kSchemaVersionKey, StateManager::kCurrentSchemaVersion, nullptr);
    state.setProperty("legacySampleFolder", lastLoadedFolder.getFullPathName(), nullptr);
    samplerEngine.serializeMicTrims(state);
    return presetManager.savePreset(name, category, state);
}

bool DrumTechProcessor::loadPresetByName(const juce::String& name)
{
    if (!presetManager.loadPreset(name))
        return false;

    auto presetState = presetManager.getCurrentState();
    if (!presetState.isValid())
        return false;

    presetState = StateManager::migrateStateIfNeeded(presetState);
    apvts.replaceState(presetState);
    samplerEngine.deserializeMicTrims(presetState);

    const auto folderPath = presetState.getProperty("legacySampleFolder").toString();
    if (folderPath.isNotEmpty())
    {
        juce::File folder(folderPath);
        if (folder.exists())
            loadSamplesFromFolder(folder);
    }
    return true;
}

bool DrumTechProcessor::enqueueUiMidiNote(int midiNote, int velocity) noexcept
{
    if (midiNote < 0 || midiNote > 127)
        return false;

    const auto vel = static_cast<juce::uint8>(juce::jlimit(1, 127, velocity));
    const uint32_t write = uiMidiWrite.load(std::memory_order_relaxed);
    const uint32_t read = uiMidiRead.load(std::memory_order_acquire);
    const uint32_t nextWrite = (write + 1u) % kUiMidiQueueSize;

    if (nextWrite == read)
        return false;

    uiMidiQueue[write] = QueuedNote { static_cast<juce::uint8>(midiNote), vel };
    uiMidiWrite.store(nextWrite, std::memory_order_release);
    return true;
}

int DrumTechProcessor::drainUiMidiToBuffer(juce::MidiBuffer& midiMessages, int numSamples) noexcept
{
    int mergedEvents = 0;
    uint32_t read = uiMidiRead.load(std::memory_order_relaxed);
    const uint32_t write = uiMidiWrite.load(std::memory_order_acquire);

    while (read != write)
    {
        const auto queued = uiMidiQueue[read];
        midiMessages.addEvent(juce::MidiMessage::noteOn(1, queued.note, queued.velocity), 0);
        ++mergedEvents;
        read = (read + 1u) % kUiMidiQueueSize;
    }

    uiMidiRead.store(read, std::memory_order_release);
    juce::ignoreUnused(numSamples);
    return mergedEvents;
}

juce::AudioProcessorValueTreeState::ParameterLayout DrumTechProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "masterLevel", "Master Level", 0.0f, 1.0f, 0.8f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "transportTempo", "Transport Tempo",
        juce::NormalisableRange<float>(30.0f, 300.0f, 0.1f), 120.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        "transportPlaying", "Transport Playing", false));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        "transportLoop", "Transport Loop", false));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        "transportRecord", "Transport Record", false));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        "groovePlaybackEnabled", "Groove Playback Enabled", false));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "triggerGlobalThreshold", "Trigger Global Threshold", 0.0f, 1.0f, 0.3f));
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("samplerDrummerProfile", 1),
        "Sampler Drummer Profile",
        juce::StringArray({"Default", "DrummerA", "DrummerB"}),
        0));
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("samplerPlayingStyle", 1),
        "Sampler Playing Style",
        juce::StringArray({"Auto", "Sticks", "Brushes"}),
        0));

    for (int channel = 0; channel < 8; ++channel)
    {
        const auto id = "triggerCh" + std::to_string(channel) + "Threshold";
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id, 1), "Trigger Ch " + juce::String(channel + 1) + " Threshold",
            0.0f, 1.0f, 0.3f));
    }

    for (int channel = 0; channel < MicBus::count; ++channel)
    {
        const auto base = "mixCh" + std::to_string(channel);
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(base + "Level", 1),
            "Mixer Ch " + juce::String(channel + 1) + " Level", 0.0f, 1.0f, 0.8f));
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(base + "Pan", 1),
            "Mixer Ch " + juce::String(channel + 1) + " Pan",
            juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f));
        parameters.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(base + "Mute", 1),
            "Mixer Ch " + juce::String(channel + 1) + " Mute", false));
        parameters.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(base + "Solo", 1),
            "Mixer Ch " + juce::String(channel + 1) + " Solo", false));

        for (int send = 0; send < MixerChannel::NUM_SEND_SLOTS; ++send)
        {
            parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(base + "Send" + std::to_string(send), 1),
                "Mixer Ch " + juce::String(channel + 1) + " Send " + juce::String(send + 1),
                0.0f, 1.0f, 0.0f));
        }
    }

    static const float eqDefaultFreqs[8] = { 80.f, 200.f, 500.f, 1000.f, 2500.f, 5000.f, 10000.f, 14000.f };
    static const int eqDefaultTypes[8]   = { 1, 0, 0, 0, 0, 0, 0, 2 };
    juce::StringArray eqTypeNames({ "Bell", "Low Shelf", "High Shelf", "Low Pass", "High Pass" });

    for (int ch = 0; ch < MicBus::count; ++ch)
    {
        for (int b = 0; b < 8; ++b)
        {
            const juce::String pb = "eqCh" + juce::String(ch) + "Band" + juce::String(b);
            juce::NormalisableRange<float> freqRange(20.0f, 20000.0f, 1.0f);
            freqRange.setSkewForCentre(1000.0f);

            parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(pb + "Freq", 2), pb + " Freq", freqRange, eqDefaultFreqs[b]));

            parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(pb + "Gain", 2), pb + " Gain",
                juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));

            parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(pb + "Q", 2), pb + " Q",
                juce::NormalisableRange<float>(0.1f, 24.0f, 0.01f, 0.35f), 1.0f));

            parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID(pb + "Type", 2), pb + " Type", eqTypeNames, eqDefaultTypes[b]));

            parameters.push_back(std::make_unique<juce::AudioParameterBool>(
                juce::ParameterID(pb + "Bypass", 2), pb + " Bypass", false));
        }
    }

    return { parameters.begin(), parameters.end() };
}

void DrumTechProcessor::applyMixerParameters()
{
    for (int channel = 0; channel < static_cast<int>(mixerChannels.size()); ++channel)
    {
        auto* mixerChannel = getMixerChannelForInput(channel);
        if (mixerChannel == nullptr)
            continue;

        const auto base = "mixCh" + std::to_string(channel);
        if (auto* level = apvts.getRawParameterValue(base + "Level"))
            mixerChannel->setGain(level->load());
        if (auto* pan = apvts.getRawParameterValue(base + "Pan"))
            mixerChannel->setPan(pan->load());
        if (auto* mute = apvts.getRawParameterValue(base + "Mute"))
            mixerChannel->setMute(mute->load() > 0.5f);
        if (auto* solo = apvts.getRawParameterValue(base + "Solo"))
            mixerChannel->setSolo(solo->load() > 0.5f);

        for (int send = 0; send < MixerChannel::NUM_SEND_SLOTS; ++send)
        {
            if (auto* sendValue = apvts.getRawParameterValue(base + "Send" + std::to_string(send)))
                mixerChannel->setSendLevel(send, sendValue->load());
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumTechProcessor();
}
