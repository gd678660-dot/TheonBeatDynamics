#include "PluginProcessor.h"
#include "PluginEditor.h"

TheonBeatDynamicsAudioProcessor::TheonBeatDynamicsAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)
                        .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), true))
{
    apvts = std::make_unique<juce::AudioProcessorValueTreeState> (*this, nullptr, "Parameters", createParameterLayout());
}

TheonBeatDynamicsAudioProcessor::~TheonBeatDynamicsAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout TheonBeatDynamicsAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterChoice> ("KEY", "Key",
        juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("SCALE", "Scale",
        juce::StringArray { "Major", "Minor", "Chromatic" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("TUNE_SPEED", "Retune Speed", 0.0f, 100.0f, 15.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("CORRECTION", "Correction", 0.0f, 100.0f, 90.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("THRESHOLD", "Threshold", -60.0f, 0.0f, -20.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RATIO", "Ratio", 1.0f, 20.0f, 4.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ATTACK", "Attack", 1.0f, 100.0f, 12.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RELEASE", "Release", 10.0f, 1000.0f, 160.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("AIR_FRESH", "Fresh Air", 0.0f, 18.0f, 6.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("MAKEUP", "Makeup Gain", -12.0f, 24.0f, 0.0f));

    return { params.begin(), params.end() };
}

void TheonBeatDynamicsAudioProcessor::prepareToPlay (double sampleRate, int)
{
    highPassL.setCoefficients (juce::IIRCoefficients::makeHighPass (sampleRate, 80.0));
    highPassR.setCoefficients (juce::IIRCoefficients::makeHighPass (sampleRate, 80.0));

    airShelfL.setCoefficients (juce::IIRCoefficients::makeHighShelf (sampleRate, 10000.0, 1.5));
    airShelfR.setCoefficients (juce::IIRCoefficients::makeHighShelf (sampleRate, 10000.0, 1.5));

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

    // 1. Detección de escala desde Sidechain (el beat)
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

    // 2. Filtros HighPass y Fresh Air
    float airGain = *apvts->getRawParameterValue ("AIR_FRESH");
    float gainLinear = juce::Decibels::decibelsToGain (airGain);
    airShelfL.setCoefficients (juce::IIRCoefficients::makeHighShelf (getSampleRate(), 10000.0, gainLinear));
    airShelfR.setCoefficients (juce::IIRCoefficients::makeHighShelf (getSampleRate(), 10000.0, gainLinear));

    if (buffer.getNumChannels() > 0)
    {
        highPassL.processSamples (buffer.getWritePointer (0), buffer.getNumSamples());
        airShelfL.processSamples (buffer.getWritePointer (0), buffer.getNumSamples());
    }
    if (buffer.getNumChannels() > 1)
    {
        highPassR.processSamples (buffer.getWritePointer (1), buffer.getNumSamples());
        airShelfR.processSamples (buffer.getWritePointer (1), buffer.getNumSamples());
    }

    // 3. AutoTune / Pitch Shifting
    float correction = *apvts->getRawParameterValue ("CORRECTION") * 0.01f;
    int numSamples = buffer.getNumSamples();
    int bufLen = pitchBuffer.getNumSamples();

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* channelData = buffer.getWritePointer (ch);
        auto* pData = pitchBuffer.getWritePointer (ch);

        for (int i = 0; i < numSamples; ++i)
        {
            pData[writePosition] = channelData[i];

            readPhase += 0.005f * (1.0f + 0.06f * correction);
            if (readPhase >= 1.0f) readPhase -= 1.0f;
            int readIdx = (writePosition - (int)(readPhase * 400.0f) + bufLen) % bufLen;

            float pitchedSample = pData[readIdx];
            channelData[i] = channelData[i] * (1.0f - correction * 0.4f) + pitchedSample * (correction * 0.4f);
        }
    }
    writePosition = (writePosition + numSamples) % bufLen;

    // Ganancia final
    float makeupDb = *apvts->getRawParameterValue ("MAKEUP");
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
    if (auto* param = apvts->getParameter ("KEY"))
        param->setValueNotifyingHost ((float)maxIndex / 11.0f);
    if (auto* param = apvts->getParameter ("SCALE"))
        param->setValueNotifyingHost (1.0f / 2.0f);
}

void TheonBeatDynamicsAudioProcessor::calibrateVocalPreset()
{
    if (auto* p = apvts->getParameter ("AIR_FRESH")) p->setValueNotifyingHost (8.5f / 18.0f);
    if (auto* p = apvts->getParameter ("THRESHOLD")) p->setValueNotifyingHost ((-24.0f + 60.0f) / 60.0f);
    if (auto* p = apvts->getParameter ("ATTACK"))    p->setValueNotifyingHost (12.0f / 100.0f);
    if (auto* p = apvts->getParameter ("TUNE_SPEED")) p->setValueNotifyingHost (10.0f / 100.0f);
    if (auto* p = apvts->getParameter ("CORRECTION")) p->setValueNotifyingHost (0.92f);
}

juce::AudioProcessorEditor* TheonBeatDynamicsAudioProcessor::createEditor()
{
    return new TheonBeatDynamicsAudioProcessorEditor (*this);
}

bool TheonBeatDynamicsAudioProcessor::hasEditor() const { return true; }
const juce::String TheonBeatDynamicsAudioProcessor::getName() const { return "TheonBeat Dynamics"; }
bool TheonBeatDynamicsAudioProcessor::acceptsMidi() const { return false; }
bool TheonBeatDynamicsAudioProcessor::producesMidi() const { return false; }
bool TheonBeatDynamicsAudioProcessor::isMidiEffect() const { return false; }
double TheonBeatDynamicsAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int TheonBeatDynamicsAudioProcessor::getNumPrograms() { return 1; }
int TheonBeatDynamicsAudioProcessor::getCurrentProgram() { return 0; }
void TheonBeatDynamicsAudioProcessor::setCurrentProgram (int) {}
const juce::String TheonBeatDynamicsAudioProcessor::getProgramName (int) { return {}; }
void TheonBeatDynamicsAudioProcessor::changeProgramName (int, const juce::String&) {}

void TheonBeatDynamicsAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts->copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TheonBeatDynamicsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts->state.getType()))
            apvts->replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheonBeatDynamicsAudioProcessor();
}
