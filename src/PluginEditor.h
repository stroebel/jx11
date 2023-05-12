/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

//#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>

#include "PluginProcessor.h"
#include "RotaryKnob.h"

using APVTS = juce::AudioProcessorValueTreeState;
using SliderAttachment = APVTS::SliderAttachment;
using ButtonAttachment = APVTS::ButtonAttachment;

//==============================================================================
/**
 */
class JX11AudioProcessorEditor : public juce::AudioProcessorEditor
{
  public:
    JX11AudioProcessorEditor(JX11AudioProcessor &);
    ~JX11AudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

  private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JX11AudioProcessor &audioProcessor;

    RotaryKnob outputLevelKnob;
    SliderAttachment outputLevelAttachment{
        audioProcessor.apvts, ParameterID::outputLevel.getParamID(), outputLevelKnob.slider};

    RotaryKnob filterResoKnob;
    SliderAttachment filterResoAttachment{
        audioProcessor.apvts, ParameterID::filterReso.getParamID(), filterResoKnob.slider};

    juce::TextButton polyModeButton;
    ButtonAttachment polyModeAttachment{audioProcessor.apvts, ParameterID::polyMode.getParamID(),
                                        polyModeButton};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JX11AudioProcessorEditor)
};
