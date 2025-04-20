#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), forwardFFT(processorRef.fftOrder), window(1 << processorRef.fftOrder, juce::dsp::WindowingFunction<float>::hann)
{
    setSize (600, 450);
    startTimerHz(30);

    addAndMakeVisible(levelMeter);
    addAndMakeVisible(spectrumAnalyzer);

    // Threshold slider
    thresholdSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "threshold", thresholdSlider);
    thresholdSlider.setSliderStyle(juce::Slider::LinearVertical);
    thresholdSlider.setRange(levelMeter.mindB, levelMeter.maxdB, 0.01f);
    thresholdSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    thresholdSlider.addListener(this);
    thresholdSlider.setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(&thresholdSlider);
    thresholdSlider.setValue(processorRef.apvts.getRawParameterValue("threshold")->load());
    levelMeter.setThreshold(thresholdSlider.getValue());

    // Cutoff slider
    cutoffSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "cutoff", cutoffSlider);
    cutoffSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    cutoffSlider.setRange(spectrumAnalyzer.minHz, spectrumAnalyzer.maxHz, 0.1f);
    cutoffSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    cutoffSlider.addListener(this);
    cutoffSlider.setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(&cutoffSlider);
    cutoffSlider.setValue(processorRef.apvts.getRawParameterValue("cutoff")->load());
    spectrumAnalyzer.setCutoff(cutoffSlider.getValue());

    // Bypass button
    bypassButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processorRef.apvts, "bypass", bypassButton);
    bypassButton.setLookAndFeel(&lookAndFeel);
    bypassButton.onClick = [this]() {
		levelMeter.setBypass(bypassButton.getToggleState());
        spectrumAnalyzer.setBypass(bypassButton.getToggleState());
	};
    addAndMakeVisible(&bypassButton);
    bypassButton.setToggleState(processorRef.apvts.getRawParameterValue("bypass")->load(), juce::NotificationType::dontSendNotification);

    // Spectrum analyzer parameters
    spectrumAnalyzer.sampleRate = processorRef.getSampleRate();
    spectrumAnalyzer.fftSize = processorRef.fftSize;

    // Shadow and light
    shadow = juce::DropShadow(juce::Colour::fromRGBA(0x00, 0x00, 0x00, 0x66), 15, juce::Point<int>(5, 5));
    light = juce::DropShadow(juce::Colour::fromRGBA(0x48, 0x47, 0x4D, 0x20), 15, juce::Point<int>(-5, -5));
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setColour(juce::Colour::fromRGB(0x18, 0x17, 0x1D));
    g.fillAll();

    // Draw title
    g.setColour(juce::Colour::fromRGB(0xF6, 0xEF, 0xDE));
    g.setFont(typeface);
    g.setFont(42.0f);
    g.drawText("There will be blood", getLocalBounds().getCentreX() - 150, 10, 300, 50, juce::Justification::centred);
    g.setOpacity(1.0f);

    // Draw shadow and light for the components
    shadow.drawForRectangle(g, levelMeter.getBounds());
    light.drawForRectangle(g, levelMeter.getBounds());
    shadow.drawForRectangle(g, spectrumAnalyzer.getBounds());
    light.drawForRectangle(g, spectrumAnalyzer.getBounds());

    bypassButton.repaint();
}

void AudioPluginAudioProcessorEditor::resized()
{
    // Level meter
    juce::Rectangle<int> levelMeterBounds(getLocalBounds());
    levelMeterBounds.setY(60);
    levelMeterBounds.setHeight(200);
    levelMeterBounds = levelMeterBounds.reduced(20);
    levelMeter.setBounds(levelMeterBounds);

    // Spectrum analyzer
    juce::Rectangle<int> spectrumAnalyserBounds = getLocalBounds();
    spectrumAnalyserBounds.setY(levelMeterBounds.getBottom() + 10);
    spectrumAnalyserBounds.setHeight(200);
    spectrumAnalyserBounds = spectrumAnalyserBounds.reduced(20);
    spectrumAnalyzer.setBounds(spectrumAnalyserBounds);

    // Sliders and buttons
    thresholdSlider.setBounds(levelMeterBounds.getX() + 60, levelMeterBounds.getY() + 20, 15, levelMeterBounds.getHeight() - 40);
    cutoffSlider.setBounds(spectrumAnalyserBounds.getCentreX() - spectrumAnalyserBounds.getWidth() / 3.0f, spectrumAnalyserBounds.getBottom() - 60, 2.0f * spectrumAnalyserBounds.getWidth() / 3.0f, 15);
    bypassButton.setBounds(20, 15, 40, 40);
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
	// Get the rms value from the audio processor
    float dryRmsValue = juce::jlimit(levelMeter.mindB, levelMeter.maxdB, processorRef.getRmsValue(true));
    float wetRmsValue = juce::jlimit(levelMeter.mindB, levelMeter.maxdB, processorRef.getRmsValue(false));

	// add the rms value to the level meter
	levelMeter.addRmsValues(dryRmsValue, wetRmsValue);

    // Perform fft
    window.multiplyWithWindowingTable(processorRef.dryFftData, processorRef.fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(processorRef.dryFftData);
    window.multiplyWithWindowingTable(processorRef.wetFftData, processorRef.fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(processorRef.wetFftData);

    // interpolate fft data
    dryLagrangeInterpolator.process(ratio, processorRef.dryFftData, dryInterpolatedFftData, INTERPOLATIONSIZE);
    wetLagrangeInterpolator.process(ratio, processorRef.wetFftData, wetInterpolatedFftData, INTERPOLATIONSIZE);

    // Update the spectrum analyzer
    spectrumAnalyzer.updateSpectra(dryInterpolatedFftData, wetInterpolatedFftData, INTERPOLATIONSIZE);
    processorRef.nextDryFFTBlockReady = false;
    processorRef.nextWetFFTBlockReady = false;

    repaint();
}

void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &thresholdSlider) {
        levelMeter.setThreshold(thresholdSlider.getValue());
        processorRef.setThreshold(thresholdSlider.getValue());
    }
    else if (slider == &cutoffSlider) {
		spectrumAnalyzer.setCutoff(cutoffSlider.getValue());
        processorRef.setCutoff(cutoffSlider.getValue());
	}
}
