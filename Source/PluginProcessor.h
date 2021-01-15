/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class PluginTemplateAudioProcessor : public juce::AudioProcessor,
									 public juce::ValueTree::Listener
{
public:
    //==============================================================================
    PluginTemplateAudioProcessor();
    ~PluginTemplateAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	// DSP Essential Functions
	void init(); // Initialize DSP values
	void prepare(double sampleRate, int samplesPerBlock); // Prepare DSP with sample rate and buffer size
	void update(); // Update DSP when user changes parameters
	void reset() override; //Clears all values out of DSP processes

	juce::AudioProcessorValueTreeState apvts;
	juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
	
	juce::AudioSampleBuffer delayBuffer;
	float* delayData; // Our circular buffer of samples
	int delayBufferLength; // Length of our delay buffer
	int writePointer; // Write pointer into delay buffer
	float phase; // Current LFO phase (Between 0-1)
	float inverseSampleRate; // 1/f_s, where f_s= sampleRate

	// User adjustable parameters
	float frequency_; // Frequency of LFO
	float sweepWidth_; // Width of LFO in samples

private:

	bool mustUpdateProcessing{ false };
	bool isActive{ false };

	void valueTreePropertyChanged(juce::ValueTree & tree, const juce::Identifier& property) override {
		// Detect when a user changes a parameter
		mustUpdateProcessing = true;
	}

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginTemplateAudioProcessor)
};
