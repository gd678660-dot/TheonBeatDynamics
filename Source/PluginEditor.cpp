#include "PluginProcessor.h"
#include "PluginEditor.h"

TheonBeatDynamicsAudioProcessorEditor::TheonBeatDynamicsAudioProcessorEditor (TheonBeatDynamicsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (800, 600);

    // Configurar botón 15s Voice Preset
    addAndMakeVisible (voiceAiButton);
    voiceAiButton.onClick = [this]()
    {
        if (!isListeningVoice)
        {
            isListeningVoice = true;
            listenSeconds = 15;
            startTimer (1000);
        }
    };

    // Configurar botón Detectar Escala
    addAndMakeVisible (detectScaleButton);
    detectScaleButton.onClick = [this]()
    {
        audioProcessor.detectScaleFromBeat();
    };
}

TheonBeatDynamicsAudioProcessorEditor::~TheonBeatDynamicsAudioProcessorEditor()
{
    stopTimer();
}

void TheonBeatDynamicsAudioProcessor{
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
    // Fondo oscuro estilo TheonBeat Dynamics
    g.fillAll (juce::Colour (0xff0c0d11));

    // Logo y textos
    g.setColour (juce::Colours::white);
    g.setFont ((20.0f, juce::Font::bold));Text ("THEONBEAT", 16, 14, 130, 24,g.setColour (juce::Colour (0xff00d2ff)); // Cyan
    g.drawText ("DYNAMICS", 146, 14, 110, 24, juce::Justification::left);

    g.setColour (juce::Colours::grey);
    g.setFont (juce::FontOptions (12.0f));
    g.drawText ("v1.1.0 (VST3)", 260, 18, 90, 20, juce::Justification::left);
}

void TheonBeatDynamicsAudioProcessorEditor::resized()
{
    voiceAiButton.setBounds (440, 48, 140, 26);
    detectScaleButton.setBounds (590, 48, 130, 26);
}
