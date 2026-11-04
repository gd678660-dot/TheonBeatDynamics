#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class TheonBeatDynamicsAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                              public juce::Timer
{
public:
    explicit TheonBeatDynamicsAudioProcessorEditor (TheonBeatDynamicsAudioProcessor&);
    ~TheonBeatDynamicsAudioProcessorEditor() override;

    void paint (override;:
    TheonBeatDynamicsAudioProcessor& audioProcessor;

    juce::TextButton voiceAiButton;
    juce::TextButton detectScaleButton;

    int listenSeconds { 15 };
    bool isListeningVoice { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheonBeatDynamicsAudioProcessorEditor)
};
