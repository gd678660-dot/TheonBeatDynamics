#include "PluginProcessor.h"
#include "PluginEditor.h"

TheonBeatDynamicsAudioProcessorEditor::TheonBeatDynamicsAudioProcessorEditor (TheonBeatDynamicsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (800, 600);

    addAndMakeVisible (voiceAiButton);
    voiceAiButton.addListener (this);

    addAndMakeVisible (detectScaleButton);
    detectScaleButton.addListener (this);
}

TheonBeatDynamicsAudioProcessorEditor::~TheonBeatDynamicsAudioProcessorEditor()
{
    voiceAiButton.removeListener (this);
    detectScaleButton.removeListener (this);
    stopTimer();
}

void TheonBeatDynamicsAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button == &voiceAiButton)
    {
        if (!isListeningVoice)
        {
            isListeningVoice = true;
            listenSeconds = 15;
            startTimer (1000);
        }
    }
    else if (button == &detectScaleButton)
    {
        audioProcessor.detectScaleFromBeat();
    }
}

void TheonBeatDynamicsAudioProcessorEditor::timerCallback()
{
    listenSeconds--;
    voiceAiButton.setButtonText ("ESCUCHANDO (" + juce::String (listenSeconds) + "s)");

    if (listenSeconds <= 0)
    {
        stopTimer();
        isListeningVoice = false;
        voiceAiButton.setButtonText ("15S VOICE PRESET");
        audioProcessor.calibrateVocalPreset();
    }
}

void TheonBeatDynamicsAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0c0d11));

    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawText ("THEONBEAT", 16, 14, 130, 24, juce::Justification::left);

    g.setColour (juce::Colour (0xff00d2ff));
    g.drawText ("DYNAMICS", 146, 14, 110, 24, juce::Justification::left);

    g.setColour (juce::Colours::grey);
    g.setFont (12.0f);
    g.drawText ("v1.1.0 (VST3)", 260, 18, 90, 20, juce::Justification::left);
}

void TheonBeatDynamicsAudioProcessorEditor::resized()
{
    voiceAiButton.setBounds (440, 48, 140, 26);
    detectScaleButton.setBounds (590, 48, 130, 26);
}
