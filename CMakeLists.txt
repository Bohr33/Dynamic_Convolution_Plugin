cmake_minimum_required(VERSION 3.22)

project(DynamicConvolver VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(JUCE)

juce_add_plugin(DynamicConvolver
    VERSION "2.0.0"
    COMPANY_NAME "BWPlugins"
    BUNDLE_ID "com.BWPlugins.DynamicConvolver"

    PLUGIN_MANUGFACTURER_CODE Bdub
    PLUGIN_CODE DyCo

    FORMATS AU VST3

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    PRODUCT_NAME "DynamicConvolver"
    AU_MAIN_TYPE "kAudioUnitType_Effect"

    COPY_PLUGIN_AFTER_BUILD FALSE
    )


juce_generate_juce_header(DynamicConvolver)

# ============================================================
# Source Files
# ============================================================


target_sources(DynamicConvolver PRIVATE

    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h

    Source/DynamicConvolver.cpp
    Source/DynamicConvolver.h

    Source/DynamicConvolutionEffect.cpp
    Source/DynamicConvolutionEffect.h

    Source/GUI/FileHighlight.cpp
    Source/GUI/FileHighlight.h
    Source/GUI/Graphics.h
    Source/GUI/Graphics.cpp
    Source/GUI/GUIComponents.h
    Source/GUI/GUIComponents.cpp

)


target_include_directories(DynamicConvolver PRIVATE
    Source
)


# ============================================================
# Compile Definitions
# ============================================================

target_compile_definitions(DynamicConvolver PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)


target_link_libraries(DynamicConvolver PRIVATE
    
    # copied from PVExperiments, double check which are
    # actually necessary
    juce::juce_dsp
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_audio_basics
    juce::juce_audio_utils
)