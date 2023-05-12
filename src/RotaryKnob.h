/*
  ==============================================================================

    RotaryKnob.h
    Created: 5 May 2023 7:14:40pm
    Author:  Jaco Stroebel

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
 */
class RotaryKnob : public juce::Component
{
  public:
    juce::Slider slider;
    juce::String label;

    RotaryKnob();
    ~RotaryKnob() override;

    void paint(juce::Graphics &) override;
    void resized() override;

  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnob)
};
