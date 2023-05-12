/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JX11AudioProcessorEditor::JX11AudioProcessorEditor(JX11AudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    outputLevelKnob.label = "Level";
    addAndMakeVisible(outputLevelKnob);

    filterResoKnob.label = "Reso";
    addAndMakeVisible(filterResoKnob);

    polyModeButton.setButtonText("Poly");
    polyModeButton.setClickingTogglesState(true);
    addAndMakeVisible(polyModeButton);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(600, 400);
}

JX11AudioProcessorEditor::~JX11AudioProcessorEditor() {}

//==============================================================================
void JX11AudioProcessorEditor::paint(juce::Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void JX11AudioProcessorEditor::resized()
{
    juce::Rectangle r(20, 20, 100, 120);
    outputLevelKnob.setBounds(r);

    r = r.withX(r.getRight() + 20);
    filterResoKnob.setBounds(r);

    polyModeButton.setSize(80, 30);
    polyModeButton.setCentrePosition(r.withX(r.getRight()).getCentre());
}
