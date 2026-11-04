#include "PluginProcessor.h"
#include "PluginEditor.h"

TheonBeatDynamicsAudioProcessorEditor::TheonBeatDynamicsAudioProcessorEditor (TheonBeatDynamicsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (800, 600);

    // Botón de 15s AI Voice Preset
    addAndMakeVisible (voiceAiButton);
    voiceAiButton.onClick = [this]() {
        if (!isListeningVoice) {
            isListeningVoice = true;
            listenSeconds = 15;
            startTimer (1000);
        }
    };

    // Botón Detector de Escala
    addAndMakeVisible (detectScaleButton);
    detectScaleButton.onClick = [this]() {
        audioProcessor.detectScaleFromBeat();
    };
}

TheonBeatDynamicsAudioProcessorEditor::~TheonBeatDynamicsAudioProcessorEditor()
{
    stopTimer();
}

void TheonBeatDynamicsAudioProcessorEditor::timerCallback()
{
    listenSeconds--;
    voiceAiButton.setButtonText ("ESCUCHANDO (" + juce::String (listenSeconds) + "s)");

    if (listenSeconds <= 0) {
        stopTimer();
        isListeningVoice = false;
        voiceAiButton.setButtonText ("15S VOICE PRESET");
        audioProcessor.calibrateVocalPreset();
    }
}

void TheonBeatDynamicsAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0c0d11)); // Fondo oscuro estudio
}

void TheonBeatDynamicsAudioProcessorEditor::resized()
{
    voiceAiButton.setBounds (420, 52, 130, 24);
    detectScaleButton.setBounds (560, 52, 120, 24);
}
