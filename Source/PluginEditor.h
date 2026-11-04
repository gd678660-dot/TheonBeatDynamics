#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

struct StudioKnobLookAndFeel : public ju{
    juce::Colour accentColour { 0xff00d4ff };

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juoverride).toFloat().reduced (4);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto center = bounds.getCentre();

        g.setColour (juce::Colour (0xff14171d));
        g.fillEllipse (bounds);
        g.setColour (juce::Colour (0xff282d37));
        g.drawEllipse (bounds, 1.5f);

        juce::Path arc;
        arc.addCentredArc (center.x, center.y, radius - 2.5f, radius - 2.5f, 0.0f,
                           rotaryStartAngle, rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle), true);
        g.setColour (accentColour);
        g.strokePath (arc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

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
explicit TheonBeatDynamicsAudioProcessorEditor (TheonBeatDynamicsAudioProcessor& p)
        : juce::AudioProcessorEditor (&p), processorRef (p)
    {
        setSize (960, 540);
        orangeLaf.accentColour = juce::Colour (0xffff7700);

        auto initKnob = [this] (juce::Slider& sl, juce::LookAndFeel* laf, double minVal, double maxVal, double defVal, double step) {
            sl.setSliderStyle (sl.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 65, 18);
            sl.setLookAndFeel (laf);
            sl.setRange (minVal, maxVal, step);
            sl.setValue (defVal);
            addAndMakeVisible (sl);
        };

        initKnob (inGainKnob,  &knobLaf,   -24.0,  24.0,   1.8, 0.1);
        initKnob (threshKnob,  &knobLaf,   -60.0,   0.0, -21.5, 0.1);
        initKnob (ratioKnob,   &knobLaf,     1.0,  20.0,   5.5, 0.1);
        initKnob (attackKnob,  &knobLaf,     0.1, 100.0,  12.0, 0.1);
        initKnob (releaseKnob, &knobLaf,    10.0, 1000.0, 160.0, 1.0);
        initKnob (makeupKnob,  &knobLaf,   -12.0,  24.0,   3.2, 0.1);
        initKnob (mixKnob,     &orangeLaf,   0.0, 100.0, 100.0, 1.0);
        initKnob (driveKnob,   &orangeLaf,   0.0,  12.0,   2.1, 0.1);
        initKnob (ceilingKnob, &orangeLaf, -12.0,   0.0,  -0.3, 0.1);

        addAndMakeVisible (clipperBtn);
        clipperBtn.setToggleState (true, juce::dontSendNotification);

        addAndMakeVisible (bypassBtn);
        bypassBtn.setToggleState (false, juce::dontSendNotification);

        startTimerHz (30);
    }

    ~TheonBeatDynamicsAudioProcessorEditor() override
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

    void timerCallback() override
    {
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff0d0f13));

        // Header
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
        g.drawText ("THEONBEAT", 20, 10, 140, 30, juce::Justification::left);
        g.setColour (juce::Colour (0xff00d4ff));
        g.drawText ("DYNAMICS", 165, 10, 140, 30, juce::Justification::left);

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

        // Meters
        g.setFont (ju(11.0f, juce::Font::bold)); (juce::Colours::lightgrey);
        g.drawText ("INPUT", 30, 320, 60, 15, juce::Justification::left);
        g.drawText ("GR", 30, 345, 60, 15, juce::Justification::left);
        g.drawText ("OUTPUT", 30, 370, 60, 15, juce::Justification::left);

        auto drawLedBar = [&g] (int x, int y, int w, int h, float valNorm, juce::Colour fillCol) {
            int numSegments = 32;
            int activeSegments = juce::jlimit (0, numSegments, static_cast<int> (valNorm * numSegments));
            float segWidth = static_cast<float> (w) / numSegments;
            for (int i = 0; i < numSegments; ++i)
            {
                auto col = (i < activeSegments) ? fill(0xff1c2128););
                g.fillRect (static_cast<float> (x) + i * segWidth, static_cast<float> (y), segWidth - 1.5f, static_cast<float> (h));
            }
        };

        drawLedBar (95, 320, 520, 14, processorRef.meterInL.load(), juce::Colour (0xff00e676));
        drawLedBar (95, 345, 520, 14, std::abs (processorRef.meterGR.load()) / 20.0f, juce::Colour (0xffff1744));
        drawLedBar (95, 370, 520, 14, processorRef.meterOutL.load(), juce::Colour (0xff00e676));

        // Screen Boxes
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

        // Botón central: LISTEN (15s)
        auto learnBounds = juce::Rectangle<int> (290, 408, 140, 38);
        g.setColour (ju(0xff00d4ff));(), 5.0f);
        g.setColour (juce::Colours::black);
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("LISTEN VOICE (15s)", learnBounds, juce::Justification::centred);

        // Detector central: ESCALA
        auto scaleBounds = juce::Rectangle<int> (290, 452, 140, 38);
        g.setColour (ju(0xff12151c));toFloat(), 5.0f);
        g.setColour (juce::Colour (0xff00d4ff));
        g.drawRoundedRectangle (scaleBounds.toFloat(), 5.0f, 1.2f);
        g.setFont (juce::FontOptions (10.0f, jug.drawText ("SCALE: D MINOR (AUTO)", scaleBounds,// Footer logo
        g.setColour (juce::Colour (0xff00d4ff));
        g.drawText ("TheonBeat", getWidth() / 2 - 50, getHeight() - 25, 100, 20, ju}

    void resized() override
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

private:
    TheonBeatDynamicsAudioProcessor& processorRef;
    StudioKnobLookAndFeel knobLaf, orangeLaf;

    juce::Slider inGainKnob, threshKnob, ratioKnob, attackKnob, releaseKnob;
    juce::Slider makeupKnob, mixKnob, driveKnob, ceilingKnob;
    juce::ToggleButton clipperBtn { "CLIPPER ON" }, bypassBtn { "BYPASS" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheonBeatDynamicsAudioProcessorEditor)
};
