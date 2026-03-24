cmake_minimum_required(VERSION 3.22)
project(DrumLibrary VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Standalone JUCE 8.0.4
add_subdirectory(/tmp/JUCE-8.0.4 jucelib)

juce_add_plugin(DrumLibrary
    COMPANY_NAME            "DrumLib"
    PLUGIN_MANUFACTURER_CODE Dlib
    PLUGIN_CODE             Dlbp
    FORMATS                 AU VST3 Standalone
    PRODUCT_NAME            "Drum Library"
    BUNDLE_ID               "com.drumlib.plugin"
    AU_MAIN_TYPE            kAudioUnitType_MusicDevice
    IS_SYNTH                TRUE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES         "Instrument" "Drums"
)

target_sources(DrumLibrary PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SamplerEngine.cpp
)

target_compile_definitions(DrumLibrary PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_COREGRAPHICS_DRAW_ASYNC=0
)

target_link_libraries(DrumLibrary PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
