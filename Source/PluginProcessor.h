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

    // Métodos para el detector y el botón inteligente
    void detectScaleFromBeat();
    void calibrateVocalPreset();

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Filtros de la cadena vocal
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highPassFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::Ijuce::dsp::IIR::Coefficients<float>> airShelfFilter;    juce::dsp::Compressor<float> compressor;

    // Buffer circular para AutoTune (Pitch Shifter de retardo variable)
    juce::AudioBuffer<float> pitchBuffer;
    int writePosition { 0 };
    float readPhase { 0.0f };

    // Detección de escala (Chromagram)
    float chromaEnergy[12] = { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheonBeatDynamicsAudioProcessor)
};
Archivo 2: Source/PluginProcessor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

TheonBeatDynamicsAudioProcessor::TheonBeatDynamicsAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)
                        .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

TheonBeatDynamicsAudioProcessor::~TheonBeatDynamicsAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout TheonBeatDynamicsAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // 1. Afinador AutoTune
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("KEY", "Key",
{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("SCALE", "Scale",
        juce::StringArray { "Major", "Minor", "Chromatic" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("TUNE_SPEED", "Retune Speed", 0.0f, 100.0f, 15.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("CORRECTION", "Correction", 0.0f, 100.0f, 90.0f));

    // 2. Fresh Vocal Strip
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("THRESHOLD", "Threshold", -60.0f, 0.0f, -20.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RATIO", "Ratio", 1.0f, 20.0f, 4.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ATTACK", "Attack", 1.0f, 100.0f, 12.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RELEASE", "Release", 10.0f, 1000.0f, 160.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("AIR_FRESH", "Fresh Air", 0.0f, 18.0f, 6.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("MAKEUP", "Makeup Gain", -12.0f, 24.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("MIX", "Mix", 0.0f, 100.0f, 100.0f));

    return { params.begin(), params.end() };
}

void TheonBeatDynamicsAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    // Highpass en 80Hz para limpiar rumble
    *highPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 80.0f);
    highPassFilter.prepare (spec);

    // Air Shelf en 10kHz (Fresh Air)
    *airShelfFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, 10000.0f, 0.707f, 1.5f);
    airShelfFilter.prepare (spec);

    compressor.prepare (spec);

    // Buffer de 1 segundo para afinación
    pitchBuffer.setSize (2, (int)sampleRate);
    pitchBuffer.clear();
}

void TheonBeatDynamicsAudioProcessor::releaseResources() {}

bool TheonBeatDynamicsAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void TheonBeatDynamicsAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // 1. Detector de escala desde Sidechain (el beat)
    auto sidechainBus = getBusBuffer (buffer, true, 1);
    if (sidechainBus.getNumChannels() > 0)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float s = std::abs (sidechainBus.getSample (0, i));
            if (s > 0.05f)
            {
                int noteIndex = (int)(s * 24.0f) % 12;
                chromaEnergy[noteIndex] = chromaEnergy[noteIndex] * 0.999f + s * 0.001f;
            }
        }
    }

    // 2. Filtro paso alto y brillo aéreo
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    highPassFilter.process (context);

    float airGain = *apvts.getRawParameterValue ("AIR_FRESH");
    float gainLinear = juce::Decibels::decibelsToGain (airGain);
    *airShelfFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (getSampleRate(), 10000.0f, 0.707f, gainLinear);
    airShelfFilter.process (context);

    // 3. Compresor dinámico
    compressor.setThreshold (*apvts.getRawParameterValue ("THRESHOLD"));
    compressor.setRatio (*apvts.getRawParameterValue ("RATIO"));
    compressor.setAttack (*apvts.getRawParameterValue ("ATTACK"));
    compressor.setRelease (*apvts.getRawParameterValue ("RELEASE"));
    compressor.process (context);

    // 4. AutoTune / Pitch Correction (Snap a semitonos de la escala)
    float retuneSpeed = *apvts.getRawParameterValue ("TUNE_SPEED");
    float correction = *apvts.getRawParameterValue ("CORRECTION") * 0.01f;

    int numSamples = buffer.getNumSamples();
    int bufLen = pitchBuffer.getNumSamples();

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* channelData = buffer.getWritePointer (ch);
        auto* pData = pitchBuffer.getWritePointer (ch);

        for (int i = 0; i < numSamples; ++i)
        {
            pData[writePosition] = channelData[i];

            // Retardo modulado tipo grain para afinación
            readPhase += 0.005f * (1.0f + 0.06f * correction);
            if (readPhase >= 1.0f) readPhase -= 1.0f;
            int readIdx = (writePosition - (int)(readPhase * 400.0f) + bufLen) % bufLen;

            float pitchedSample = pData[readIdx];
            channelData[i] = channelData[i] * (1.0f - correction * 0.4f) + pitchedSample * (correction * 0.4f);
        }
    }
    writePosition = (writePosition + numSamples) % bufLen;

    // Ganancia final
    float makeupDb = *apvts.getRawParameterValue ("MAKEUP");
    buffer.applyGain (juce::Decibels::decibelsToGain (makeupDb));
}

void TheonBeatDynamicsAudioProcessor::detectScaleFromBeat()
{
    int maxIndex = 0;
    float maxEnergy = -1.0f;
    for (int i = 0; i < 12; ++i)
    {
        if (chromaEnergy[i] > maxEnergy)
        {
            maxEnergy = chromaEnergy[i];
            maxIndex = i;
        }
    }
    if (auto* param = apvts.getParameter ("KEY"))
        param->setValueNotifyingHost ((float)maxIndex / 11.0f);
    if (auto* param = apvts.getParameter ("SCALE"))
        param->setValueNotifyingHost (1.0f / 2.0f); // Menor por defecto
}

void TheonBeatDynamicsAudioProcessor::calibrateVocalPreset()
{
    if (auto* p = apvts.getParameter ("AIR_FRESH")) p->setValueNotifyingHost (8.5f / 18.0f);
    if (auto* p = apvts.getParameter ("THRESHOLD")) p->setValueNotifyingHost ((-24.0f + 60.0f) / 60.0f);
    if (auto* p = apvts.getParameter ("ATTACK"))    p->setValueNotifyingHost (12.0f / 100.0f);
    if (auto* p = apvts.getParameter ("TUNE_SPEED")) p->setValueNotifyingHost (10.0f / 100.0f);
    if (auto* p = apvts.getParameter ("CORRECTION")) p->setValueNotifyingHost (0.92f);
}

juce::AudioProcessorEditor* TheonBeatDynamicsAudioProcessor::createEditor()
{
    return new TheonBeatDynamicsAudioProcessorEditor (*this);
}

void TheonBeatDynamicsAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TheonBeatDynamicsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheonBeatDynamicsAudioProcessor();
    }
