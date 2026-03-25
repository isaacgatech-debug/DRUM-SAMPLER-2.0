cmake_minimum_required(VERSION 3.22)
project(DrumSampler2 VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
add_subdirectory(/tmp/JUCE-8.0.4 ${CMAKE_BINARY_DIR}/JUCE)

# Define plugin
juce_add_plugin(DrumSampler2
    COMPANY_NAME "Gridlock"
    PLUGIN_MANUFACTURER_CODE Dsmp
    PLUGIN_CODE Ds2p
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Drum Tech"
    BUNDLE_ID "com.gridlock.drumtech"
    
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Instrument" "Sampler"
    AU_MAIN_TYPE "kAudioUnitType_MusicDevice"
)

# Binary resources
juce_add_binary_data(DrumSampler2Resources
    SOURCES
        Resources/Backdrops/drum-backdrop.png
        "Resources/LOGO/Gridlock Logo - White.png"
)

# Source files
target_sources(DrumSampler2 PRIVATE
    # Core
    Source/Core/PluginProcessor.cpp
    Source/Core/PluginEditor.cpp
    Source/Core/StateManager.cpp
    Source/Core/ErrorLogger.h
    Source/Core/DebugConsole.h
    Source/Core/PresetManager.cpp
    Source/UI/TooltipManager.h
    Source/UI/KeyboardShortcuts.h
    Source/Testing/PerformanceMonitor.h
    Source/Testing/TestSuite.h
    
    # Sampler
    Source/Sampler/SamplerEngine.cpp
    Source/Sampler/DrumVoice.cpp
    Source/Sampler/SampleManager.cpp
    Source/Sampler/VelocityCurve.cpp
    
    # Mixer
    Source/Mixer/MixerChannel.cpp
    Source/Mixer/BusManager.cpp
    Source/Mixer/SendFXBus.cpp
    Source/Mixer/MixerUI.cpp
    
    # Effects
    Source/Effects/EffectProcessor.cpp
    Source/Effects/ReverbEffect.cpp
    Source/Effects/DelayEffect.cpp
    Source/Effects/EQEffect.cpp
    Source/Effects/CompressorEffect.cpp
    Source/Effects/TransientShaperEffect.cpp
    Source/Effects/PluginManager.cpp
    Source/Effects/ThirdPartyPluginEffect.cpp
    
    # Routing
    Source/Routing/AudioBus.cpp
    Source/Routing/RoutingMatrix.cpp
    
    # Grooves
    Source/Grooves/GrooveLibrary.cpp
    Source/Grooves/GrooveTimeline.cpp
    Source/Grooves/RhythmMatcher.cpp
    Source/Grooves/MIDIPlayer.cpp
    
    # Trigger
    Source/Trigger/AudioTriggerEngine.cpp
    Source/Trigger/OnsetDetector.cpp
    Source/Trigger/DrumClassifier.cpp
    Source/Trigger/TriggerUI.cpp
    
    # UI
    Source/UI/DrumKitView.cpp
    Source/UI/InteractiveDrumPad.cpp
    Source/UI/GrooveBrowser.cpp
    Source/UI/MixerView.cpp
    Source/UI/ChannelStrip.cpp
    Source/UI/AnalogKnob.cpp
    Source/UI/VUMeter.cpp
    Source/UI/RoutingView.cpp
    Source/UI/PluginBrowser.cpp
    Source/UI/FrequencyBandEditor.cpp
    Source/UI/GrooveTimeline.cpp
    Source/UI/TransportBar.cpp
    Source/UI/InstrumentSettingsPanel.cpp
    Source/UI/ParametricEQWindow.cpp
)

# Compile definitions
target_compile_definitions(DrumSampler2 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_COREGRAPHICS_DRAW_ASYNC=0
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_AU=1
)

# Link JUCE modules
target_link_libraries(DrumSampler2 PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_dsp
    DrumSampler2Resources
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Include binary data headers
target_include_directories(DrumSampler2 PRIVATE 
    ${CMAKE_BINARY_DIR}/juce_binarydata_DrumSampler2Resources/JuceLibraryCode
)
