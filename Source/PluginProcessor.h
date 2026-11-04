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

    const juce::String getName() const override { return "TheonBeat Dynamics"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Métodos para el detector de escala y el botón de voz
    void detectScaleFromBeat();
    void calibrateVocalPreset();

    // Sistema de gestión de parámetros
    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Filtros de la cadena vocal (Paso alto y brillo Air)
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highPassFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> airShelfFilter;
    juce::dsp::Compressor<float> compressor;

    // Buffer circular para AutoTune (Pitch Shifter de retardo variable)
    juce::AudioBuffer<float> pitchBuffer;
    int writePosition { 0 };
    float readPhase { 0.0f };

    // Detección de escala (Chromagram de 12 notas)
    float chromaEnergy[12] = { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheonBeatDynamicsAudioProcessor)
};
