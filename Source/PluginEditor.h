#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class TheonBeatDynamicsAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                              public juce::Button::Listener,
                                              public juce::Timer
{
public:
    explicit TheonBeatDynamicsAudioProcessorEditor (TheonBeatDynamicsAudioProcessor&);
    ~TheonBeatDynamicsAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void buttonClicked (juce::Button* button) override;

private:
    TheonBeatDynamicsAudioProcessor& audioProcessor;

    voiceAiButton { "15S VOICE PRESET" };Button detectScaleButton { "DETECTAR ESCALA" };

    int listenSeconds { 15 };
    bool isListeningVoice { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheonBeatDynamicsAudioProcessorEditor)
};
