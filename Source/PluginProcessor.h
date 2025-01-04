/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define NUMFILTERS 4

//==============================================================================
/**
*/
class TherewillnotbebloodAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TherewillnotbebloodAudioProcessor();
    ~TherewillnotbebloodAudioProcessor() override;

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

    //==============================================================================
    float getRmsValue(bool);
    void pushNextDrySampleIntoFifo(float);
    void pushNextWetSampleIntoFifo(float);

    //==============================================================================
    void setThreshold(float);
    void setCutoff(float);

    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    juce::AudioProcessorValueTreeState apvts;

    //==============================================================================
    enum
    {
        fftOrder = 10,
        fftSize = 1 << fftOrder
    };

    #include <vector>

    float dryFifo[fftSize];
    float wetFifo[fftSize];
    float dryFftData[2 * fftSize];
    float wetFftData[2 * fftSize];
    int dryFifoIndex = 0;
    int wetFifoIndex = 0;
    bool nextDryFFTBlockReady = false;
    bool nextWetFFTBlockReady = false;

private:
    //==============================================================================
    float dryRmsValue = 0.0f;
    float wetRmsValue = 0.0f;

    juce::dsp::Compressor<float> compressor;

    juce::dsp::StateVariableTPTFilter<float> filter[NUMFILTERS];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TherewillnotbebloodAudioProcessor)
};
