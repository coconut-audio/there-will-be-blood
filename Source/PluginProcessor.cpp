/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TherewillnotbebloodAudioProcessor::TherewillnotbebloodAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
    apvts(*this, nullptr, "PARAMETERS", createParameters())
{
    apvts.state = juce::ValueTree("PARAMETERS");
}

TherewillnotbebloodAudioProcessor::~TherewillnotbebloodAudioProcessor()
{
}

//==============================================================================
const juce::String TherewillnotbebloodAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TherewillnotbebloodAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TherewillnotbebloodAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TherewillnotbebloodAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TherewillnotbebloodAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TherewillnotbebloodAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TherewillnotbebloodAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TherewillnotbebloodAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TherewillnotbebloodAudioProcessor::getProgramName (int index)
{
    return {};
}

void TherewillnotbebloodAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TherewillnotbebloodAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    compressor.prepare(spec);
    compressor.setThreshold(*apvts.getRawParameterValue("threshold"));
    compressor.setRatio(8.0f);
    compressor.setAttack(20.0f);
    compressor.setRelease(20.0f);

    for (int i = 0; i < 4; ++i) {
        filter[i].prepare(spec);
        filter[i].setType(juce::dsp::StateVariableTPTFilterType::highpass);
        filter[i].setCutoffFrequency(*apvts.getRawParameterValue("cutoff"));
	}
}

void TherewillnotbebloodAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TherewillnotbebloodAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void TherewillnotbebloodAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get copies of the buffer
    juce::AudioSampleBuffer drySignalBuffer;
    drySignalBuffer.makeCopyOf(buffer);
    juce::AudioSampleBuffer wetSignalBuffer;
    wetSignalBuffer.makeCopyOf(buffer);

    // Apply compression
    juce::dsp::AudioBlock<float> compressorBlock(wetSignalBuffer);
    juce::dsp::ProcessContextReplacing<float> compressorContext(compressorBlock);
    compressor.process(compressorContext);

    // Get rms values
    dryRmsValue = juce::Decibels::gainToDecibels((buffer.getRMSLevel(0, 0, drySignalBuffer.getNumSamples()) + drySignalBuffer.getRMSLevel(1, 0, drySignalBuffer.getNumSamples())) / 2.0f);
    wetRmsValue = juce::Decibels::gainToDecibels((wetSignalBuffer.getRMSLevel(0, 0, wetSignalBuffer.getNumSamples()) + wetSignalBuffer.getRMSLevel(1, 0, wetSignalBuffer.getNumSamples())) / 2.0f);

    // Push samples into dry fifo
    for (int sampleIndex = 0; sampleIndex < buffer.getNumSamples(); ++sampleIndex) {
        float wetSample = 0.5f * wetSignalBuffer.getSample(0, sampleIndex) + 0.5f * wetSignalBuffer.getSample(1, sampleIndex);
        pushNextDrySampleIntoFifo(wetSample);
    }

    for (int i = 0; i < NUMFILTERS; ++i) {
		juce::dsp::AudioBlock<float> filterBlock(wetSignalBuffer);
		juce::dsp::ProcessContextReplacing<float> filterContext(filterBlock);
		filter[i].process(filterContext);
	}

    // Push samples into wet fifo
    for (int sampleIndex = 0; sampleIndex < buffer.getNumSamples(); ++sampleIndex) {
		float wetSample = 0.5f * wetSignalBuffer.getSample(0, sampleIndex) + 0.5f * wetSignalBuffer.getSample(1, sampleIndex);
		pushNextWetSampleIntoFifo(wetSample);
	}

    // Mix dry signal with phase inverted wet signal
    wetSignalBuffer.applyGain(-1.0f);
    wetSignalBuffer.addFrom(0, 0, drySignalBuffer, 0, 0, buffer.getNumSamples());
    wetSignalBuffer.addFrom(1, 0, drySignalBuffer, 1, 0, buffer.getNumSamples());

    if (*apvts.getRawParameterValue("bypass")) {
        buffer.makeCopyOf(drySignalBuffer);
    }
    else {
		buffer.makeCopyOf(wetSignalBuffer);
	}
}

//==============================================================================
bool TherewillnotbebloodAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TherewillnotbebloodAudioProcessor::createEditor()
{
    return new TherewillnotbebloodAudioProcessorEditor (*this);
}

//==============================================================================
void TherewillnotbebloodAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    std::unique_ptr <juce::XmlElement> params(apvts.state.createXml());

    copyXmlToBinary(*params, destData);

}

void TherewillnotbebloodAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr <juce::XmlElement> params(getXmlFromBinary(data, sizeInBytes));

    if (params != nullptr) {
        if (params->hasTagName(apvts.state.getType())) {
            apvts.state = juce::ValueTree::fromXml(*params);
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout TherewillnotbebloodAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", juce::NormalisableRange<float>(- 60.0f, 36.0f), -0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("cutoff", "Cutoff", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 4000.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));

    return layout;
}

float TherewillnotbebloodAudioProcessor::getRmsValue(bool drySignal)
{
    if (drySignal) {
		return dryRmsValue;
	}
	else {
		return wetRmsValue;
	}
}

void TherewillnotbebloodAudioProcessor::pushNextDrySampleIntoFifo(float sample)
{
    if (dryFifoIndex == fftSize) {
        if (!nextDryFFTBlockReady) {
            juce::zeromem(dryFftData, sizeof(dryFftData));
            memcpy(dryFftData, dryFifo, sizeof(dryFifo));
            nextDryFFTBlockReady = true;
        }

        dryFifoIndex = 0;
    }

    dryFifo[dryFifoIndex++] = sample;
}

void TherewillnotbebloodAudioProcessor::pushNextWetSampleIntoFifo(float sample)
{
    if (wetFifoIndex == fftSize) {
        if (!nextWetFFTBlockReady) {
            juce::zeromem(wetFftData, sizeof(wetFftData));
            memcpy(wetFftData, wetFifo, sizeof(wetFifo));
            nextWetFFTBlockReady = true;
        }

        wetFifoIndex = 0;
    }

    wetFifo[wetFifoIndex++] = sample;
}

void TherewillnotbebloodAudioProcessor::setThreshold(float threshold)
{
	compressor.setThreshold(threshold);
}

void TherewillnotbebloodAudioProcessor::setCutoff(float cutoff)
{
    for (int i = 0; i < NUMFILTERS; ++i) {
        filter[i].setCutoffFrequency(cutoff);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new TherewillnotbebloodAudioProcessor();
}