/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#define _USE_MATH_DEFINES
#include <math.h>

//==============================================================================
PluginTemplateAudioProcessor::PluginTemplateAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
	  apvts(*this, nullptr, "Parameters", createParameters()), delayBuffer(2, 1)
#endif
{
	init();
	apvts.state.addListener(this);
}

PluginTemplateAudioProcessor::~PluginTemplateAudioProcessor()
{
	
}

//==============================================================================
const juce::String PluginTemplateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginTemplateAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginTemplateAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginTemplateAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginTemplateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginTemplateAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginTemplateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PluginTemplateAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PluginTemplateAudioProcessor::getProgramName (int index)
{
    return {};
}

void PluginTemplateAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PluginTemplateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	prepare(sampleRate, samplesPerBlock);
	update();
	reset();
	isActive = true;

	
}

void PluginTemplateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginTemplateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PluginTemplateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// If prepareToPlay is not called yet, do not run DSP
	if (!isActive) {
		return;
	}

	if (mustUpdateProcessing) {
		update();
	}

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
	auto numSamples = buffer.getNumSamples();

    // DSP Processing
	int dpw;
	float ph;
	float width;

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
		float* delayData = delayBuffer.getWritePointer(channel);
		dpw = writePointer;
		ph = phase;
		width = sweepWidth_;
        
		for (int sample = 0; sample < numSamples; ++sample) {
			
			const float in = channelData[sample];
			float interpolatedSample = 0.0f;

			// Calculate the position of read pointer with respect to write pointer
			float currentDelay = width * (0.5f + 0.5f * sinf(2.0 * M_PI * ph));
			float dpr = fmodf((float)dpw - 
									  (float)(currentDelay * getSampleRate()) + 
									  (float)delayBufferLength - 3.0, (float)delayBufferLength);

			// Linear Interpolation
			float fraction = dpr - floorf(dpr);
			int previousSample = (int)floorf(dpr);
			int nextSample = (previousSample + 1) % delayBufferLength;
			interpolatedSample = fraction * delayData[nextSample] + (1.0f - fraction) * delayData[previousSample];

			// Store current information in delay buffer
			delayData[dpw] = in;

			// Increment write pointer at constant rate depending on parameters.
			if (++dpw >= delayBufferLength) {
				dpw = 0;
			}

			// Store the output sample into output buffer to get the vibrato effect.
			channelData[sample] = interpolatedSample;

			// Update LFO Phase
			ph += frequency_ * inverseSampleRate;
			if (ph >= 1.0) {
				ph -= 1.0;
			}
		}
    }

	writePointer = dpw;
	phase = ph;
	

	// Clear Buffer
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());
}

//==============================================================================
bool PluginTemplateAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginTemplateAudioProcessor::createEditor()
{
    return new PluginTemplateAudioProcessorEditor(*this);
}

//==============================================================================
void PluginTemplateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

	juce::ValueTree copyState = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml = copyState.createXml();
	copyXmlToBinary(*xml.get(), destData);
}

void PluginTemplateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

	std::unique_ptr<juce::XmlElement> xml = getXmlFromBinary(data, sizeInBytes);
	juce::ValueTree copyState = juce::ValueTree::fromXml(*xml.get());
	apvts.replaceState(copyState);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginTemplateAudioProcessor();
}

void PluginTemplateAudioProcessor::init() {

	// Set default values:
	sweepWidth_ = 0.01f;
	frequency_ = 2.0f;

	delayBufferLength = 1;
	phase = 0.0f;

	// Start the circular buffer pointer at the beginning
	writePointer = 0;
	
	
}

void PluginTemplateAudioProcessor::prepare(double sampleRate, int samplesPerBlock) {

	inverseSampleRate = 1.0f / sampleRate;

	float maxWidth = 0.05f;
	delayBufferLength = (int)(maxWidth * sampleRate) + 3;
	delayBuffer.setSize(2, delayBufferLength);
	delayBuffer.clear();
	phase = 0.0;
}

void PluginTemplateAudioProcessor::update() {

	// Once updates made, change update processing variable back to false
	mustUpdateProcessing = false;
	auto freq = apvts.getRawParameterValue("FREQ");
	auto width = apvts.getRawParameterValue("WIDTH");

	frequency_ = *freq;
	sweepWidth_ = *width;
}

void PluginTemplateAudioProcessor::reset() {
	delayBuffer.clear();
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginTemplateAudioProcessor::createParameters() {

	std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
	
	// Lambda functions for parameter construction
	std::function<juce::String(float, int)> valueToTextFunction = [](float x, int l) {
		return juce::String(x, 4);
	};
	std::function<float (const juce::String &)> textToValueFunction = [](const juce::String & str) {
		return str.getFloatValue();
	};
	
	// Create parameters here

	// Frequency parameter
	parameters.push_back(std::make_unique<juce::AudioParameterFloat>("FREQ", "Frequency",
		juce::NormalisableRange<float>(1.0f, 12.0f), 6.0f, "Hz",
		juce::AudioProcessorParameter::genericParameter,
		valueToTextFunction, textToValueFunction));

	// Sweep width parameter
	parameters.push_back(std::make_unique<juce::AudioParameterFloat>("WIDTH", "Width",
		juce::NormalisableRange<float>(0.01f, 0.05f), 0.03f, "",
		juce::AudioProcessorParameter::genericParameter,
		valueToTextFunction, textToValueFunction));


	return { parameters.begin(), parameters.end() };
}
