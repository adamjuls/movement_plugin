/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PluginTemplateAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PluginTemplateAudioProcessorEditor (PluginTemplateAudioProcessor&);
    ~PluginTemplateAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginTemplateAudioProcessor& audioProcessor;

	std::unique_ptr<juce::Slider> frequencySlider, widthSlider;
	std::unique_ptr<juce::Label> frequencyLabel, widthLabel;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencyAttachment, widthAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginTemplateAudioProcessorEditor)
};
