#include "PluginProcessor.h"
#include "PluginEditor.h"

Processor::Processor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
    apvts(*this, nullptr, "PARAMETERS", createParameters())
{
    apvts.state = ValueTree("PARAMETERS");
}

Processor::~Processor()
{
}

const String Processor::getName() const
{
    return JucePlugin_Name;
}

bool Processor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Processor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Processor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Processor::getTailLengthSeconds() const
{
    return 0.0;
}

int Processor::getNumPrograms()
{
    return 1;
}

int Processor::getCurrentProgram()
{
    return 0;
}

void Processor::setCurrentProgram (int index)
{
    ignoreUnused (index);
}

const String Processor::getProgramName (int index)
{
    ignoreUnused (index);
    return {};
}

void Processor::changeProgramName (int index, const String& newName)
{
    ignoreUnused (index, newName);
}

void Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = uint32(samplesPerBlock);
    spec.numChannels = uint32(getTotalNumOutputChannels());

    compressor.prepare(spec);
    compressor.setThreshold(*apvts.getRawParameterValue("threshold"));
    compressor.setRatio(8.0f);
    compressor.setAttack(20.0f);
    compressor.setRelease(20.0f);

    for (int i = 0; i < 4; ++i) {
        filter[i].prepare(spec);
        filter[i].setType(dsp::StateVariableTPTFilterType::highpass);
        filter[i].setCutoffFrequency(*apvts.getRawParameterValue("cutoff"));
	}
}

void Processor::releaseResources()
{
}

bool Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else

    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void Processor::processBlock (AudioBuffer<float>& buffer,
                                              MidiBuffer& /*midiMessages*/)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get copies of the buffer
    AudioSampleBuffer drySignalBuffer;
    drySignalBuffer.makeCopyOf(buffer);
    AudioSampleBuffer wetSignalBuffer;
    wetSignalBuffer.makeCopyOf(buffer);

    // Apply compression
    dsp::AudioBlock<float> compressorBlock(wetSignalBuffer);
    dsp::ProcessContextReplacing<float> compressorContext(compressorBlock);
    compressor.process(compressorContext);

    // Get rms values
    dryRmsValue = Decibels::gainToDecibels((buffer.getRMSLevel(0, 0, drySignalBuffer.getNumSamples()) + drySignalBuffer.getRMSLevel(1, 0, drySignalBuffer.getNumSamples())) / 2.0f);
    wetRmsValue = Decibels::gainToDecibels((wetSignalBuffer.getRMSLevel(0, 0, wetSignalBuffer.getNumSamples()) + wetSignalBuffer.getRMSLevel(1, 0, wetSignalBuffer.getNumSamples())) / 2.0f);

    // Push samples into dry fifo
    for (int sampleIndex = 0; sampleIndex < buffer.getNumSamples(); ++sampleIndex) {
        float wetSample = 0.5f * wetSignalBuffer.getSample(0, sampleIndex) + 0.5f * wetSignalBuffer.getSample(1, sampleIndex);
        pushNextDrySampleIntoFifo(wetSample);
    }

    for (int i = 0; i < NUMFILTERS; ++i) {
        dsp::AudioBlock<float> filterBlock(wetSignalBuffer);
        dsp::ProcessContextReplacing<float> filterContext(filterBlock);
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

bool Processor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* Processor::createEditor()
{
    return new Editor (*this);
}

void Processor::getStateInformation (MemoryBlock& destData)
{
    std::unique_ptr <XmlElement> params(apvts.state.createXml());

    copyXmlToBinary(*params, destData);
}

void Processor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr <XmlElement> params(getXmlFromBinary(data, sizeInBytes));

    if (params != nullptr) {
        if (params->hasTagName(apvts.state.getType())) {
            apvts.state = ValueTree::fromXml(*params);
        }
    }
}

AudioProcessorValueTreeState::ParameterLayout Processor::createParameters()
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<AudioParameterFloat>("threshold", "Threshold", NormalisableRange<float>(- 60.0f, 36.0f), -0.0f));
    layout.add(std::make_unique<AudioParameterFloat>("cutoff", "Cutoff", NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 4000.0f));
    layout.add(std::make_unique<AudioParameterBool>("bypass", "Bypass", false));

    return layout;
}

float Processor::getRmsValue(bool drySignal)
{
    if (drySignal) {
		return dryRmsValue;
	}
	else {
		return wetRmsValue;
	}
}

void Processor::pushNextDrySampleIntoFifo(float sample)
{
    if (dryFifoIndex == fftSize) {
        if (!nextDryFFTBlockReady) {
            zeromem(dryFftData, sizeof(dryFftData));
            memcpy(dryFftData, dryFifo, sizeof(dryFifo));
            nextDryFFTBlockReady = true;
        }

        dryFifoIndex = 0;
    }

    dryFifo[dryFifoIndex++] = sample;
}

void Processor::pushNextWetSampleIntoFifo(float sample)
{
    if (wetFifoIndex == fftSize) {
        if (!nextWetFFTBlockReady) {
            zeromem(wetFftData, sizeof(wetFftData));
            memcpy(wetFftData, wetFifo, sizeof(wetFifo));
            nextWetFFTBlockReady = true;
        }

        wetFifoIndex = 0;
    }

    wetFifo[wetFifoIndex++] = sample;
}

void Processor::setThreshold(float threshold)
{
	compressor.setThreshold(threshold);
}

void Processor::setCutoff(float cutoff)
{
    for (int i = 0; i < NUMFILTERS; ++i) {
        filter[i].setCutoffFrequency(cutoff);
    }
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Processor();
}
