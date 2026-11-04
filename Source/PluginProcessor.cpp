#include "PluginProcessor.h"
#include "PluginEditor.h"

TheonBeatDynamicsAudioProcessor::TheonBeatDynamicsAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    addParameter (paramInputGain = new juce::AudioParameterFloat ("inGain", "Input Gain", -24.0f, 24.0f, 1.8f));
    addParameter (paramThreshold = new juce::AudioParameterFloat ("thresh", "Threshold", -60.0f, 0.0f, -21.5f));
    addParameter (paramRatio = new juce::AudioParameterFloat ("ratio", "Ratio", 1.0f, 20.0f, 5.5f));
    addParameter (paramAttack = new juce::AudioParameterFloat ("attack", "Attack", 0.1f, 100.0f, 12.0f));
    addParameter (paramRelease = new juce::AudioParameterFloat ("release", "Release", 10.0f, 1000.0f, 160.0f));
    addParameter (paramMakeup = new juce::AudioParameterFloat ("makeup", "Makeup", -12.0f, 24.0f, 3.2f));
    addParameter (paramMix = new juce::AudioParameterFloat ("mix", "Mix", 0.0f, 100.0f, 100.0f));
    addParameter (paramDrive = new juce::AudioParameterFloat ("drive", "Drive", 0.0f, 12.0f, 2.1f));
    addParameter (paramClipperOn = new juce::AudioParameterBool ("clipper", "Clipper ON", true));
    addParameter (paramCeiling = new juce::AudioParameterFloat ("ceiling", "Ceiling", -12.0f, 0.0f, -0.3f));
    addParameter (paramBypass = new juce::AudioParameterBool ("bypass", "Bypass", false));
}

TheonBeatDynamicsAudioProcessor::~TheonBeatDynamicsAudioProcessor() {}

void TheonBeatDynamicsAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/) {
    currentSampleRate = sampleRate;
}

void TheonBeatDynamicsAudioProcessor::releaseResources() {}

void TheonBeatDynamicsAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    if (paramBypass->get()) return;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float inGainLin = juce::Decibels::decibelsToGain<float> (paramInputGain->get());
    buffer.applyGain (inGainLin);

    float inLevel = buffer.getMagnitude (0, buffer.getNumSamples());
    meterInL.store (inLevel);

    float threshDb = paramThreshold->get();
    float inDb = juce::Decibels::gainToDecibels<float> (juce::jmax (inLevel, 0.00001f));
    float grDb = 0.0f;

    if (inDb > threshDb) {
        float ratio = paramRatio->get();
        float excess = inDb - threshDb;
        grDb = -excess * (1.0f - (1.0f / ratio));
    }
    meterGR.store (grDb);

    float compGainLin = juce::Decibels::decibelsToGain<float> (grDb);
    buffer.applyGain (compGainLin);

    float makeupLin = juce::Decibels::decibelsToGain<float> (paramMakeup->get());
    buffer.applyGain (makeupLin);

    if (paramClipperOn->get()) {
        float ceilLin = juce::Decibels::decibelsToGain<float> (paramCeiling->get());
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            auto* channelData = buffer.getWritePointer (ch);
            for (int s = 0; s < buffer.getNumSamples(); ++s) {
                channelData[s] = juce::jlimit (-ceilLin, ceilLin, channelData[s]);
            }
        }
    }

    meterOutL.store (buffer.getMagnitude (0, buffer.getNumSamples()));
}

juce::AudioProcessorEditor* TheonBeatDynamicsAudioProcessor::createEditor() {
    return new TheonBeatDynamicsAudioProcessorEditor (*this);
}

void TheonBeatDynamicsAudioProcessor::getStateInformation (juce::MemoryBlock&) {}
void TheonBeatDynamicsAudioProcessor::setStateInformation (const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new TheonBeatDynamicsAudioProcessor();
}

