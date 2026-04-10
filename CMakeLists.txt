cmake_minimum_required(VERSION 3.22)
project(DrumTech VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE 8.0.4: override with -DJUCE_PATH=/path/to/JUCE or place a copy at ThirdParty/JUCE
set(_drumtech_juce_dir "")
if(DEFINED JUCE_PATH AND IS_DIRECTORY "${JUCE_PATH}" AND EXISTS "${JUCE_PATH}/CMakeLists.txt")
    set(_drumtech_juce_dir "${JUCE_PATH}")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/JUCE/CMakeLists.txt")
    set(_drumtech_juce_dir "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/JUCE")
endif()

set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Build JUCE examples" FORCE)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Build JUCE extras" FORCE)

if(_drumtech_juce_dir)
    add_subdirectory("${_drumtech_juce_dir}" "${CMAKE_BINARY_DIR}/JUCE-build")
else()
    include(FetchContent)
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(juce)
endif()

# Define plugin
juce_add_plugin(DrumTech
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
juce_add_binary_data(DrumTechResources
    SOURCES
        "Resources/Backdrops/Drum Tech.jpg"
        "Resources/Backdrops/Drum Tech (1).jpg"
        "Resources/LOGO/Gridlock Logo - White.png"
        "Resources/Icons/home.png"
        "Resources/Icons/kitbuilder.png"
        "Resources/Icons/mixer.png"
        "Resources/Icons/trigger.png"
        "Resources/Icons/settings.png"
)

# Source files
target_sources(DrumTech PRIVATE
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
    Source/Effects/ParametricEQEffect.cpp
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
    Source/UI/DrumTechLookAndFeel.cpp
    Source/UI/SettingsView.cpp
    Source/UI/InstrumentSettingsPanel.cpp
    Source/UI/ParametricEQWindow.cpp
)

# Compile definitions
target_compile_definitions(DrumTech PUBLIC
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
target_link_libraries(DrumTech PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_dsp
    DrumTechResources
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Include binary data headers
target_include_directories(DrumTech PRIVATE 
    ${CMAKE_BINARY_DIR}/juce_binarydata_DrumTechResources/JuceLibraryCode
)
