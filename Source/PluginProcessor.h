#pragma once
#include <JuceHeader.h>

class TheonBeatDynamicsAudioProcessorEditor;

class TheonBeatDynamicsAudioProcessor : public juce::AudioProcessor
{
public:
    TheonBeatDynamicsAudioProcessor();
    ~TheonBeatDynamicsAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "TheonBeat Dynamics"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Métricas en tiempo real para la pantalla y vúmetros
    std::atomic<float> meterInL { 0.0f };
    std::atomic<float> meterOutL { 0.0f };
    std::atomic<float> meterGR { 0.0f };

    // Parámetros del plugin
    juce::AudioParameterFloat* paramInputGain = nullptr;
    juce::AudioParameterFloat* paramThreshold = nullptr;
    juce::AudioParameterFloat* paramRatio = nullptr;
    juce::AudioParameterFloat* paramAttack = nullptr;
    juce::AudioParameterFloat* paramRelease = nullptr;
    juce::AudioParameterFloat* paramMakeup = nullptr;
    juce::AudioParameterFloat* paramMix = nullptr;
    juce::AudioParameterFloat* paramDrive = nullptr;
    juce::AudioParameterBool*  paramClipperOn = nullptr;
    juce::AudioParameterFloat* paramCeiling = nullptr;
    juce::AudioParameterBool*  paramBypass = nullptr;

private:
    double currentSampleRate = 44100.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheonBeatDynamicsAudioProcessor)
};
