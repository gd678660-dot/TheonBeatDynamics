cmake_minimum_required(VERSION 3.20)
project(TheonBeatDynamics VERSION 1.1.0)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(TheonBeatDynamics
    COMPANY_NAME "TheonBeat"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    FORMATS VST3
    PRODUCT_NAME "TheonBeat Dynamics"
)

target_sources(TheonBeatDynamics PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
)

target_compile_definitions(TheonBeatDynamics PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(TheonBeatDynamics PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
)
