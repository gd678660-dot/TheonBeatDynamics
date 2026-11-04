#include "PluginEditor.h"
#include "PluginProcessor.h"

TheonBeatDynamicsAudioProcessorEditor::TheonBeatDynamicsAudioProcessorEditor (TheonBeatDynamicsAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (960, 540);
    orangeLaf.accentColour =(0xffff7700);::Slider& sl, juce::LookAndFeel* laf) {
        sl.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        sl.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 65, 18);
        sl.setLookAndFeel (laf);
        addAndMakeVisible (sl);
    };

    setupSlider (inGainKnob,  &knobLaf);
    setupSlider (threshKnob,  &knobLaf);
    setupSlider (ratioKnob,   &knobLaf);
    setupSlider (attackKnob,  &knobLaf);
    setupSlider (releaseKnob, &knobLaf);
    setupSlider (makeupKnob,  &knobLaf);
    setupSlider (mixKnob,     &orangeLaf);
    setupSlider (driveKnob,   &orangeLaf);
    setupSlider (ceilingKnob, &orangeLaf);

    inGainKnob.setRange (-24.0, 24.0, 0.1);
    inGainKnob.setValue (processorRef.paramInputGain->get());
    inGainKnob.onValueChange = [this] { *processorRef.paramInputGain = static_cast<float> (inGainKnob.getValue()); };

    threshKnob.setRange (-60.0, 0.0, 0.1);
    threshKnob.setValue (processorRef.paramThreshold->get());
    threshKnob.onValueChange = [this] { *processorRef.paramThreshold = static_cast<float> (threshKnob.getValue()); };

    ratioKnob.setRange (1.0, 20.0, 0.1);
    ratioKnob.setValue (processorRef.paramRatio->get());
    ratioKnob.onValueChange = [this] { *processorRef.paramRatio = static_cast<float> (ratioKnob.getValue()); };

    attackKnob.setRange (0.1, 100.0, 0.1);
    attackKnob.setValue (processorRef.paramAttack->get());
    attackKnob.onValueChange = [this] { *processorRef.paramAttack = static_cast<float> (attackKnob.getValue()); };

    releaseKnob.setRange (10.0, 1000.0, 1.0);
    releaseKnob.setValue (processorRef.paramRelease->get());
    releaseKnob.onValueChange = [this] { *processorRef.paramRelease = static_cast<float> (releaseKnob.getValue()); };

    makeupKnob.setRange (-12.0, 24.0, 0.1);
    makeupKnob.setValue (processorRef.paramMakeup->get());
    makeupKnob.onValueChange = [this] { *processorRef.paramMakeup = static_cast<float> (makeupKnob.getValue()); };

    mixKnob.setRange (0.0, 100.0, 1.0);
    mixKnob.setValue (processorRef.paramMix->get());
    mixKnob.onValueChange = [this] { *processorRef.paramMix = static_cast<float> (mixKnob.getValue()); };

    driveKnob.setRange (0.0, 12.0, 0.1);
    driveKnob.setValue (processorRef.paramDrive->get());
    driveKnob.onValueChange = [this] { *processorRef.paramDrive = static_cast<float> (driveKnob.getValue()); };

    ceilingKnob.setRange (-12.0, 0.0, 0.1);
    ceilingKnob.setValue (processorRef.paramCeiling->get());
    ceilingKnob.onValueChange = [this] { *processorRef.paramCeiling = static_cast<float> (ceilingKnob.getValue()); };

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
    inGainKnob.setLookAndFeel (nullptr);
    threshKnob.setLookAndFeel (nullptr);
    ratioKnob.setLookAndFeel (nullptr);
    attackKnob.setLookAndFeel (nullptr);
    releaseKnob.setLookAndFeel (nullptr);
    makeupKnob.setLookAndFeel (nullptr);
    mixKnob.setLookAndFeel (nullptr);
    driveKnob.setLookAndFeel (nullptr);
    ceilingKnob.setLookAndFeel (nullptr);
}

void TheonBeatDynamicsAudioProcessorEditor::mouseUp (const juce::MouseEvent& e)
{
    // Clic en el botón central LISTEN VOICE (15S)
    auto learnBtnBounds = juce::Rectangle<int> (290, 408, 140, 38);
    if (learnBtnBounds.contains (e.getPosition()))
    {
        processorRef.startVocalLearn();
    }
}

void TheonBeatDynamicsAudioProcessorEditor::timerCallback()
{
    if (!processorRef.isLearning.load())
    {
        if (threshKnob.getValue() != processorRef.paramThreshold->get())
            threshKnob.setValue (processorRef.paramThreshold->get(), juce::dontSendNotification);
        if (makeupKnob.getValue() != processorRef.paramMakeup->get())
            makeupKnob.setValue (processorRef.paramMakeup->get(), juce::dontSendNotification);
    }
    repaint();
}

void TheonBeatDynamicsAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0d0f13));

    // Cabecera
    g.setColour (juce::g.setFont (juce::FontOptions (22.0f, jug.drawText ("THEONBEAT", 20, 10, 140, 30, juce::Justification::left);
    g.setColour (juce::Colour (0xff00d4ff));
    g.drawText ("DYNAMICS", 165, 10, 140, 30, juce::Justification::left);

    // Módulos principales con marcos oscuros
    auto drawSection = [&g] (jur, const juce::String& title) {(0xff13161c));toFloat(), 6.0f);
        g.setColour (juce::Colour (0xff252a35));
        g.drawRoundedRectangle (r.toFloat(), 6.0f, 1.0f);
        g.setColour (juce::Colours::grey);
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (title, r.getX(), r.getY() + 5, r.getWidth(), 16,};

    drawSection ({ 15, 50, 140, 230 }, "INPUT / DETECTOR");
    drawSection ({ 165, 50, 560, 230 }, "COMPRESSOR / EXPANDER");
    drawSection ({ 735, 50, 210, 230 }, "GAIN / MIX");

    drawSection ({ 15, 290, 710, 220 }, "DISPLAY / METERING");
    drawSection ({ 735, 290, 210, 220 }, "LIM / CLIPPER");

    // Vúmetros LED
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.setColour (juce::Colours::lightgrey);
    g.drawText ("INPUT", 30, 320, 60, 15, juce::g.drawText ("GR", 30, 345, 60, 15, juce::Justification::left);
    g.drawText ("OUTPUT", 30, 370, 60, 15, juce::Justification::left);

    auto drawLedBar = [&g] (int x, int y, int w, int h, float valNorm, juce::Colour fillCol) {
        int numSegments = 32;
        int activeSegments = juce::jlimit (0, numSegments, static_cast<int> (valNorm * numSegments));
        float segWidth = static_cast<float> (w) / numSegments;
        for (int i = 0; i < numSegments; ++i)
        {
            auto col = (i < activeSegments) ? fillCol : juce::Colour (0xff1c2128);
            g.setColour (col);
            g.fillRect (static_cast<float> (x) + i * segWidth, static_cast<float> (y), segWidth - 1.5f, static_cast<float> (h));
        }
    };

    drawLedBar (95, 320, 520, 14, processorRef.meterInL.load(),(0xff00e676));345, 520, 14, std::abs (processorRef.meterGR.load()) / 20.0f, juce::Colour (0xffff1744));
    drawLedBar (95, 370, 520, 14, processorRef.meterOutL.load(), juce::Colour (0xff00e676));

    // Cuadros Transfer Curve y RTA
    g.setColour (juce::Colour (0xff171a22));
    g.fillRect (60, 400, 220, 95);
    g.fillRect (440, 400, 260, 95);
    g.setColour (juce::Colour (0xff2f3542));
    g.drawRect (60, 400, 220, 95);
    g.drawRect (440, 400, 260, 95);

    g.setColour (juce::Colours::cyan);
    g.drawLine (60.0f, 495.0f, 170.0f, 440.0f, 2.0f);
    g.drawLine (170.0f, 440.0f, 280.0f, 420.0f, 2.0f);

    juce::Path rtaPath;
    rtaPath.startNewSubPath (440.0f, 495.0f);
    rtaPath.lineTo (480.0f, 470.0f);
    rtaPath.lineTo (520.0f, 430.0f);
    rtaPath.lineTo (560.0f, 450.0f);
    rtaPath.lineTo (610.0f, 420.0f);
    rtaPath.lineTo (660.0f, 460.0f);
    rtaPath.lineTo (700.0f, 495.0f);
    g.setColour (juce::Colour (0xffff7700));
    g.strokePath (rtaPath, juce::PathStrokeType (2.0f));

    // BOTÓN CENTRAL 1: LISTEN VOICE (15S)
    bool isLearning = processorRef.isLearning.load();
    auto learnBtnBounds = juce::Rectangle<int> (290, 408, 140, 38);
    g.setColour (isLearning ? juce::Colour (0xffff3d00) : juce::Colour (0xff00d4ff));
    g.fillRoundedRectangle (learnBtnBounds.toFloat(), 5.0f);
    g.setColour (g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    if (isLearning)
    {
        int pct = static_cast<int> (processorRef.learnProgress.load() * 100.0f);
        g.drawText ("LISTENING " + juce::String (pct) + "%", learnBtnBounds, juce::Justification::centred);
    }
    else
    {
        g.drawText ("LISTEN VOICE (15s)", learnBtnBounds, juce::Justification::centred);
    }

    // BOTÓN CENTRAL 2: DETECTOR DE ESCALA
    auto scaleBounds = juce::Rectangle<int> (290, 452, 140, 38);
    g.setColour (juce::Colour (0xff12151c));
    g.fillRoundedRectangle (scaleBounds.toFloat(), 5.0f);
    g.setColour (juce::Colour (0xff00d4ff));
    g.drawRoundedRectangle (scaleBounds.toFloat(), 5.0f, 1.2f);
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.drawText ("SCALE: D MINOR (AUTO)", scaleBounds, juce::Justification::centred);

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
