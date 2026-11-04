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
3. Source/PluginEditor.h
#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

struct StudioKnobLookAndFeel : public juce::LookAndFeel_V4
{
    juce::Colour accentColour { 0xff00d4ff };

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto center = bounds.getCentre();

        // Cuerpo metálico oscuro
        g.setColour (juce::Colour (0xff14171d));
        g.fillEllipse (bounds);
        g.setColour (juce::Colour (0xff282d37));
        g.drawEllipse (bounds, 1.5f);

        // Anillo exterior con arco de luz
        juce::Path arc;
        arc.addCentredArc (center.x, center.y, radius - 2.5f, radius - 2.5f, 0.0f,
                           rotaryStartAngle, rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle), true);
        g.setColour (accentColour);
        g.strokePath (arc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Puntero central
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto thumbLength = radius * 0.60f;
        g.setColour (juce::Colours::white);
        g.drawLine (center.x, center.y,
                    center.x + thumbLength * std::sin (angle),
                    center.y - thumbLength * std::cos (angle), 2.5f);
    }
};

class TheonBeatDynamicsAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit TheonBeatDynamicsAudioProcessorEditor (TheonBeatDynamicsAudioProcessor&);
    ~TheonBeatDynamicsAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    TheonBeatDynamicsAudioProcessor& processorRef;
    StudioKnobLookAndFeel knobLaf, orangeLaf;

    juce::Slider inGainKnob, threshKnob, ratioKnob, attackKnob, releaseKnob;
    juce::Slider makeupKnob, mixKnob, driveKnob, ceilingKnob;
    juce::ToggleButton clipperBtn { "CLIPPER ON" }, bypassBtn { "BYPASS" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheonBeatDynamicsAudioProcessorEditor)
};
4. Source/PluginEditor.cpp
#include "PluginEditor.h"

TheonBeatDynamicsAudioProcessorEditor::TheonBeatDynamicsAudioProcessorEditor (TheonBeatDynamicsAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (960, 540);
    orangeLaf.accentColour = juce::Colour (0xffff7700);

    auto setupKnob = [this] (juce::Slider& sl, juce::LookAndFeel* laf, const juce::String& /*text*/) {
        sl.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        sl.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 65, 18);
        sl.setLookAndFeel (laf);
        addAndMakeVisible (sl);
    };

    setupKnob (inGainKnob,  &knobLaf,   "INPUT GAIN");
    setupKnob (threshKnob,  &knobLaf,   "THRESHOLD");
    setupKnob (ratioKnob,   &knobLaf,   "RATIO");
    setupKnob (attackKnob,  &knobLaf,   "ATTACK");
    setupKnob (releaseKnob, &knobLaf,   "RELEASE");
    setupKnob (makeupKnob,  &knobLaf,   "MAKEUP GAIN");
    setupKnob (mixKnob,     &orangeLaf, "MIX");
    setupKnob (driveKnob,   &orangeLaf, "DRIVE");
    setupKnob (ceilingKnob, &orangeLaf, "CEILING");

    // Enlazar valores con el procesador
    threshKnob.setRange (-60.0, 0.0, 0.1);
    threshKnob.setValue (processorRef.paramThreshold->get());
    threshKnob.onValueChange = [this] { *processorRef.paramThreshold = (float)threshKnob.getValue(); };

    ratioKnob.setRange (1.0, 20.0, 0.1);
    ratioKnob.setValue (processorRef.paramRatio->get());
    ratioKnob.onValueChange = [this] { *processorRef.paramRatio = (float)ratioKnob.getValue(); };

    makeupKnob.setRange (-12.0, 24.0, 0.1);
    makeupKnob.setValue (processorRef.paramMakeup->get());
    makeupKnob.onValueChange = [this] { *processorRef.paramMakeup = (float)makeupKnob.getValue(); };

    addAndMakeVisible (clipperBtn);
    clipperBtn.setToggleState (processorRef.paramClipperOn->get(), juce::dontSendNotification);
    clipperBtn.onClick = [this] { *processorRef.paramClipperOn = clipperBtn.getToggleState(); };

    addAndMakeVisible (bypassBtn);
    bypassBtn.setToggleState (processorRef.paramBypass->get(), juce::dontSendNotification);
    bypassBtn.onClick = [this] { *processorRef.paramBypass = bypassBtn.getToggleState(); };

    startTimerHz (30);
}

TheonBeatDynamicsAudioProcessorEditor::~TheonBeatDynamicsAudioProcessorEditor()
{
    stopTimer();
}

void TheonBeatDynamicsAudioProcessorEditor::timerCallback()
{
    repaint();
}

void TheonBeatDynamicsAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fondo principal de la ventana
    g.fillAll (juce::Colour (0xff0d0f13));

    // Cabecera del plugin
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("THEONBEAT", 20, 10, 140, 30, juce::Justification::left);
    g.setColour (juce::Colour (0xff00d4ff));
    g.drawText ("DYNAMICS", 165, 10, 140, 30, juce::Justification::left);

    // Módulos con marcos metálicos
    auto drawSection = [&g] (juce::Rectangle<int> r, const juce::String& title) {
        g.setColour (juce::Colour (0xff13161c));
        g.fillRoundedRectangle (r.toFloat(), 6.0f);
        g.setColour (juce::Colour (0xff252a35));
        g.drawRoundedRectangle (r.toFloat(), 6.0f, 1.0f);
        g.setColour (juce::Colours::grey);
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (title, r.getX(), r.getY() + 5, r.getWidth(), 16, juce::Justification::centred);
    };

    drawSection ({ 15, 50, 140, 230 }, "INPUT / DETECTOR");
    drawSection ({ 165, 50, 560, 230 }, "COMPRESSOR / EXPANDER");
    drawSection ({ 735, 50, 210, 230 }, "GAIN / MIX");

    drawSection ({ 15, 290, 710, 220 }, "DISPLAY / METERING");
    drawSection ({ 735, 290, 210, 220 }, "LIM / CLIPPER");

    // Etiquetas y Vúmetros segmentados estilo LED bar
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.setColour (juce::Colours::lightgrey);
    g.drawText ("INPUT", 30, 320, 60, 15, juce::Justification::left);
    g.drawText ("GR", 30, 345, 60, 15, juce::Justification::left);
    g.drawText ("OUTPUT", 30, 370, 60, 15, juce::Justification::left);

    auto drawLedBar = [&g] (int x, int y, int w, int h, float valNorm, juce::Colour fillCol) {
        int numSegments = 32;
        int activeSegments = juce::jlimit (0, numSegments, static_cast<int> (valNorm * numSegments));
        float segWidth = (float)w / numSegments;
        for (int i = 0; i < numSegments; ++i)
        {
            auto col = (i < activeSegments) ? fillCol : juce::Colour (0xff1c2128);
            g.setColour (col);
            g.fillRect ((float)x + i * segWidth, (float)y, segWidth - 1.5f, (float)h);
        }
    };

    drawLedBar (95, 320, 520, 14, processorRef.meterInL.load(), juce::Colour (0xff00e676));
    drawLedBar (95, 345, 520, 14, std::abs (processorRef.meterGR.load()) / 20.0f, juce::Colour (0xffff1744));
    drawLedBar (95, 370, 520, 14, processorRef.meterOutL.load(), juce::Colour (0xff00e676));

    // Gráfica de Transfer Curve y RTA simulados en el display
    g.setColour (juce::Colour (0xff171a22));
    g.fillRect (100, 400, 240, 95);
    g.fillRect (370, 400, 320, 95);
    g.setColour (juce::Colour (0xff2f3542));
    g.drawRect (100, 400, 240, 95);
    g.drawRect (370, 400, 320, 95);

    g.setColour (juce::Colours::cyan);
    g.drawLine (100, 495, 230, 440, 2.0f);
    g.drawLine (230, 440, 340, 420, 2.0f);

    // Espectro RTA en naranja
    juce::Path rtaPath;
    rtaPath.startNewSubPath (370, 495);
    rtaPath.lineTo (410, 470);
    rtaPath.lineTo (450, 430);
    rtaPath.lineTo (500, 450);
    rtaPath.lineTo (560, 420);
    rtaPath.lineTo (620, 460);
    rtaPath.lineTo (690, 495);
    g.setColour (juce::Colour (0xffff7700));
    g.strokePath (rtaPath, juce::PathStrokeType (2.0f));

    // Logo pie de página
    g.setColour (juce::Colour (0xff00d4ff));
    g.drawText ("TheonBeat", getWidth() / 2 - 50, getHeight() - 25, 100, 20, juce::Justification::centred);
}

void TheonBeatDynamicsAudioProcessorEditor::resized()
{
    inGainKnob.setBounds (35, 85, 95, 95);

    threshKnob.setBounds (190, 85, 120, 120);
    ratioKnob.setBounds (330, 85, 85, 85);
    attackKnob.setBounds (440, 85, 85, 85);
    releaseKnob.setBounds (550, 85, 85, 85);

    makeupKnob.setBounds (795, 75, 85, 85);
    mixKnob.setBounds (795, 175, 85, 85);

    driveKnob.setBounds (795, 315, 80, 80);
    clipperBtn.setBounds (780, 400, 110, 28);
    ceilingKnob.setBounds (795, 430, 80, 80);

    bypassBtn.setBounds (20, getHeight() - 30, 90, 24);
}
