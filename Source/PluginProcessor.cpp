#include "PluginProcessor.h"

TheonBeatDynamicsAudioProcessor::TheonBeatDynamicsAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    addParameter (paramThreshold   = new juce::AudioParameterFloat ("thresh", "Threshold", -60.0f, 0.0f, -18.0f));
    addParameter (paramRatio       = new juce::AudioParameterFloat ("ratio", "Ratio", 1.0f, 20.0f, 4.0f));
    addParameter (paramAttack      = new juce::AudioParameterFloat ("attack", "Attack", 0.1f, 150.0f, 10.0f));
    addParameter (paramRelease     = new juce::AudioParameterFloat ("release", "Release", 10.0f, 1000.0f, 120.0f));
    addParameter (paramRetuneSpeed = new juce::AudioParameterFloat ("retune", "Retune Speed", 0.0f, 100.0f, 10.0f));
    addParameter (paramHumanize    = new juce::AudioParameterFloat ("humanize", "Humanize", 0.0f, 100.0f, 20.0f));
    addParameter (paramDeEsser     = new juce::AudioParameterFloat ("deess", "De-Esser", 0.0f, 100.0f, 35.0f));
    addParameter (paramMakeup      = new juce::AudioParameterFloat ("makeup", "Makeup Gain", 0.0f, 24.0f, 3.0f));
}

TheonBeatDynamicsAudioProcessor::~TheonBeatDynamicsAudioProcessor() {}

void TheonBeatDynamicsAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
}

void TheonBeatDynamicsAudioProcessor::releaseResources() {}

void TheonBeatDynamicsAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputSample = channelData[sample];
            float absSample = std::abs (inputSample);
            
            float gainReduction = 1.0f;
            float threshLinear = juce::Decibels::decibelsToGain (paramThreshold->get());
            if (absSample > threshLinear)
            {
                float excessdB = juce::Decibels::gainToDecibels (absSample) - paramThreshold->get();
                float compresseddB = excessdB / paramRatio->get();
                gainReduction = juce::Decibels::decibelsToGain (paramThreshold->get() + compresseddB) / (absSample + 0.0001f);
            }

            float makeupGain = juce::Decibels::decibelsToGain (paramMakeup->get());
            channelData[sample] = inputSample * gainReduction * makeupGain;
        }
    }
}

juce::AudioProcessorEditor* TheonBeatDynamicsAudioProcessor::createEditor() { return new juce::GenericAudioProcessorEditor (*this); }
bool TheonBeatDynamicsAudioProcessor::hasEditor() const { return true; }
const juce::String TheonBeatDynamicsAudioProcessor::getName() const { return "TheonBeat Dynamics"; }
bool TheonBeatDynamicsAudioProcessor::acceptsMidi() const { return false; }
bool TheonBeatDynamicsAudioProcessor::producesMidi() const { return false; }
double TheonBeatDynamicsAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int TheonBeatDynamicsAudioProcessor::getNumPrograms() { return 1; }
int TheonBeatDynamicsAudioProcessor::getCurrentProgram() { return 0; }
void TheonBeatDynamicsAudioProcessor::setCurrentProgram (int) {}
const juce::String TheonBeatDynamicsAudioProcessor::getProgramName (int) { return {}; }
void TheonBeatDynamicsAudioProcessor::changeProgramName (int, const juce::String&) {}
void TheonBeatDynamicsAudioProcessor::getStateInformation (juce::MemoryBlock&) {}
void TheonBeatDynamicsAudioProcessor::setStateInformation (const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheonBeatDynamicsAudioProcessor();
}
