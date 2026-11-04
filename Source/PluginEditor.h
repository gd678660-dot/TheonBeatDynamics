#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class TheonBeatDynamicsAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                              public juce::Timer
{
public:
    explicit TheonBeatDynamicsAudioProcessorEditor (TheonBeatDynamicsAudioProcessor&);
    ~TheonBeatDynamicsAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    TheonBeatDynamicsAudioProcessor& audioProcessor;

    // Botones interactivos
    juce::TextButton voiceListenButton { "15S VOICE PRESET" };
    juce::TextButton detectScaleButton { "DETECTAR ESCALA" };

    int listenSeconds = 15;
    bool isListeningVoice = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheonBeatDynamicsAudioProcessorEditor)
};
