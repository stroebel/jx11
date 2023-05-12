/*
  ==============================================================================

    RotaryKnob.cpp
    Created: 5 May 2023 7:14:40pm
    Author:  Jaco Stroebel

  ==============================================================================
*/

#include "RotaryKnob.h"

static constexpr int labelHeight = 15;
static constexpr int textBoxHeight = 20;

//==============================================================================
RotaryKnob::RotaryKnob()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, textBoxHeight);
    addAndMakeVisible(slider);

    setBounds(0, 0, 100, 120);
}

RotaryKnob::~RotaryKnob() {}

void RotaryKnob::paint(juce::Graphics &g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll(getLookAndFeel().findColour(
        juce::ResizableWindow::backgroundColourId)); // clear the background

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    auto bounds = getLocalBounds();
    g.drawText(label, juce::Rectangle<int>{0, 0, bounds.getWidth(), labelHeight},
               juce::Justification::centred);
}

void RotaryKnob::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    auto bounds = getLocalBounds();
    slider.setBounds(0, labelHeight, bounds.getWidth(), bounds.getHeight() - labelHeight);
}
