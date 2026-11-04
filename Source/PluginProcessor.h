#pragma once

#include <JuceHeader.h>

class TheonBeatDynamicsAudioProcessor  : public juce::AudioProcessor
{
public:
    TheonBeatDynamicsAudioProcessor();
    ~TheonBeatDynamicsAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const jugetName() const override { return "TheonBeat Dynamics"; } }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juoverride {}    void setStateInformation (const void* data, int sizeInBytes) override;

    void detectScaleFromBeat();
    void calibrateVocalPreset();

    juce::AudioProcessorValueTreeState apvts;

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Filtros estándar de JUCE (sin necesidad del módulo juce_dsp)
    juce::IIRFilter highPassL, highPassR;
    juce::IIRFilter airShelfL, airShelfR;

    // Buffer circular para AutoTune
    juce::AudioBuffer<float> pitchBuffer;
    int writePosition { 0 };
    float readPhase { 0.0f };

    // Detección de escala (12 notas)
    float chromaEnergy[12] = { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheonBeatDynamicsAudioProcessor)
};
