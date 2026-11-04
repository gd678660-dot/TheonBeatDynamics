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
2. Source/PluginProcessor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

TheonBeatDynamicsAudioProcessor::TheonBeatDynamicsAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    addParameter (paramInputGain = new juce::AudioParameterFloat ("inGain", "Input Gain", -24.0f, 24.0f, 1.8f));
    addParameter (paramThreshold = new juce::AudioParameterFloat ("thresh", "Threshold", -60.0f, 0.0f, -21.5f));
    addParameter (paramRatio     = new juce::AudioParameterFloat ("ratio", "Ratio", 1.0f, 20.0f, 5.5f));
    addParameter (paramAttack    = new juce::AudioParameterFloat ("attack", "Attack", 0.1f, 100.0f, 12.0f));
    addParameter (paramRelease   = new juce::AudioParameterFloat ("release", "Release", 10.0f, 1000.0f, 160.0f));
    addParameter (paramMakeup    = new juce::AudioParameterFloat ("makeup", "Makeup", -12.0f, 24.0f, 3.2f));
    addParameter (paramMix       = new juce::AudioParameterFloat ("mix", "Mix", 0.0f, 100.0f, 100.0f));
    addParameter (paramDrive     = new juce::AudioParameterFloat ("drive", "Drive", 0.0f, 12.0f, 2.1f));
    addParameter (paramClipperOn = new juce::AudioParameterBool  ("clipper", "Clipper ON", true));
    addParameter (paramCeiling   = new juce::AudioParameterFloat ("ceiling", "Ceiling", -12.0f, 0.0f, -0.3f));
    addParameter (paramBypass    = new juce::AudioParameterBool  ("bypass", "Bypass", false));
}

TheonBeatDynamicsAudioProcessor::~TheonBeatDynamicsAudioProcessor() {}

void TheonBeatDynamicsAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
}

void TheonBeatDynamicsAudioProcessor::releaseResources() {}

void TheonBeatDynamicsAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    if (paramBypass->get()) return;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // 1. Ganancia de Entrada
    float inGainLin = juce::Decibels::decibelsToGain (paramInputGain->get());
    buffer.applyGain (inGainLin);
    float inLevel = buffer.getMagnitude (0, buffer.getNumSamples());
    meterInL.store (inLevel);

    // 2. Compresión básica y cálculo de Gain Reduction
    float threshDb = paramThreshold->get();
    float inDb = juce::Decibels::gainToDecibels (juce::jmax (inLevel, 0.00001f));
    float grDb = 0.0f;
    if (inDb > threshDb)
    {
        float ratio = paramRatio->get();
        float excess = inDb - threshDb;
        grDb = -excess * (1.0f - (1.0f / ratio));
    }
    meterGR.store (grDb);

    float compGainLin = juce::Decibels::decibelsToGain (grDb);
    buffer.applyGain (compGainLin);

    // 3. Ganancia de Compensación (Makeup)
    float makeupLin = juce::Decibels::decibelsToGain (paramMakeup->get());
    buffer.applyGain (makeupLin);

    // 4. Clipper de Salida
    if (paramClipperOn->get())
    {
        float ceilLin = juce::Decibels::decibelsToGain (paramCeiling->get());
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* channelData = buffer.getWritePointer (ch);
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                channelData[s] = juce::jlimit (-ceilLin, ceilLin, channelData[s]);
            }
        }
    }

    // 5. Nivel de Salida
    meterOutL.store (buffer.getMagnitude (0, buffer.getNumSamples()));
}

juce::AudioProcessorEditor* TheonBeatDynamicsAudioProcessor::createEditor()
{
    return new TheonBeatDynamicsAudioProcessorEditor (*this);
}

void TheonBeatDynamicsAudioProcessor::getStateInformation (juce::MemoryBlock&) {}
void TheonBeatDynamicsAudioProcessor::setStateInformation (const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheonBeatDynamicsAudioProcessor();
}
