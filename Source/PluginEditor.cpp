/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginTemplateAudioProcessorEditor::PluginTemplateAudioProcessorEditor (PluginTemplateAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    
	frequencySlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
	addAndMakeVisible(frequencySlider.get());
	widthSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextBoxBelow);
	addAndMakeVisible(widthSlider.get());

	frequencyLabel = std::make_unique <juce::Label> ("", "Frequency");
	addAndMakeVisible(frequencyLabel.get());
	frequencyLabel->attachToComponent(frequencySlider.get(), false);
	frequencyLabel->setJustificationType(juce::Justification::centred);

	widthLabel = std::make_unique <juce::Label>("", "Width");
	addAndMakeVisible(widthLabel.get());
	widthLabel->attachToComponent(widthSlider.get(), false);
	widthLabel->setJustificationType(juce::Justification::centred);

	frequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "FREQ", *frequencySlider);
	widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "WIDTH", *widthSlider);

	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	setSize(500, 500);
}

PluginTemplateAudioProcessorEditor::~PluginTemplateAudioProcessorEditor()
{
}

//==============================================================================
void PluginTemplateAudioProcessorEditor::paint (juce::Graphics& g)
{
	auto bounds = getLocalBounds();
	auto textBounds = bounds.removeFromTop(40);

	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	g.fillRect(bounds);

	g.setColour(juce::Colours::black);
	g.fillRect(textBounds);

	g.setColour(juce::Colours::white);
	g.setFont(juce::Font(20.0f).withExtraKerningFactor(0.1f));
	g.drawFittedText("Movement by FASA Audio", textBounds, juce::Justification::centred, 1);
}

void PluginTemplateAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	auto bounds = getLocalBounds();
	auto textBounds = bounds.removeFromTop(40);
	bounds.reduce(40, 40);

	juce::Grid grid;
	using Track = juce::Grid::TrackInfo;
	using Fr = juce::Grid::Fr;

	// 4 evenly spaced columns
	grid.templateColumns = {
		Track(Fr(1)), Track(Fr(1))
	};

	// 2 evenly spaced columns
	grid.templateRows = {
		Track(Fr(1))
	};
	
	// Margin between each space
	grid.rowGap = juce::Grid::Px(10);
	grid.columnGap = juce::Grid::Px(10);


	grid.items.add(frequencySlider.get());
	grid.items.add(widthSlider.get());

	grid.performLayout(bounds);
}
