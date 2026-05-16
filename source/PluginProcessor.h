#pragma once

#include <JuceHeader.h>
#include <juce_audio_processors/juce_audio_processors.h>

class Processor final : public AudioProcessor
{
public:
    Processor();
    ~Processor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    using AudioProcessor::processBlock;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    float getRmsValue(bool);
    void pushNextDrySampleIntoFifo(float);
    void pushNextWetSampleIntoFifo(float);

    void setCompressorThreshold(float);
    void setFilterCutoff(float);

    AudioProcessorValueTreeState::ParameterLayout createParameters();
    AudioProcessorValueTreeState apvts;

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
    float dryRmsValue = 0.0f;
    float wetRmsValue = 0.0f;

    dsp::Compressor<float> compressor;

    const static int numFilters = 4;
    dsp::StateVariableTPTFilter<float> filter[numFilters];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};
