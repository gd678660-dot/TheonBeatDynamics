#pragma once
#include <JuceHeader.h>

class TheonBeatDynamicsAudioProcessor;

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
public:
    explicit TheonBeatDynamicsAudioProcessorEditor (TheonBeatDynamicsAudioProcessor&);
    ~TheonBeatDynamicsAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void mouseUp (const juce::MouseEvent& e) override;

private:
    TheonBeatDynamicsAudioProcessor& processorRef;
    StudioKnobLookAndFeel knobLaf, orangeLaf;

    juce::Slider inGainKnob, threshKnob, ratioKnob, attackKnob, releaseKnob;
    juce::Slider makeupKnob, mixKnob, driveKnob, ceilingKnob;
    juce::ToggleButton clipperBtn { "CLIPPER ON" }, bypassBtn { "BYPASS" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheonBeatDynamicsAudioProcessorEditor)
};
