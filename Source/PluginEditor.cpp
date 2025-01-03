/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TherewillnotbebloodAudioProcessorEditor::TherewillnotbebloodAudioProcessorEditor (TherewillnotbebloodAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), forwardFFT(10), window(1 << 10, juce::dsp::WindowingFunction<float>::hann)
{
    setSize (600, 450);
    startTimerHz(frequency);

    addAndMakeVisible(levelMeter);
    addAndMakeVisible(spectrumAnalyzer);

    // Threshold slider
    thresholdSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "threshold", thresholdSlider);
    thresholdSlider.setSliderStyle(juce::Slider::LinearVertical);
    thresholdSlider.setRange(levelMeter.mindB, levelMeter.maxdB, 0.01f);
    thresholdSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    thresholdSlider.addListener(this);
    thresholdSlider.setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(&thresholdSlider);
    thresholdSlider.setValue(audioProcessor.apvts.getRawParameterValue("threshold")->load());
    levelMeter.setThreshold(thresholdSlider.getValue());

    // Cutoff slider
    cutoffSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "cutoff", cutoffSlider);
    cutoffSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    cutoffSlider.setRange(spectrumAnalyzer.minHz, spectrumAnalyzer.maxHz, 0.1f);
    cutoffSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    cutoffSlider.addListener(this);
    cutoffSlider.setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(&cutoffSlider);
    cutoffSlider.setValue(audioProcessor.apvts.getRawParameterValue("cutoff")->load());
    spectrumAnalyzer.setCutoff(cutoffSlider.getValue());

    // Bypass button
    bypassButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "bypass", bypassButton);
    bypassButton.setLookAndFeel(&lookAndFeel);
    bypassButton.onClick = [this]() {
		levelMeter.setBypass(bypassButton.getToggleState());
        spectrumAnalyzer.setBypass(bypassButton.getToggleState());
	};
    addAndMakeVisible(&bypassButton);
    bypassButton.setToggleState(audioProcessor.apvts.getRawParameterValue("bypass")->load(), juce::NotificationType::dontSendNotification);

    // Spectrum analyzer parameters
    spectrumAnalyzer.sampleRate = audioProcessor.getSampleRate();
    spectrumAnalyzer.fftSize = audioProcessor.fftSize;

    // Shadow and light
    shadow = juce::DropShadow(juce::Colour::fromRGBA(0x00, 0x00, 0x00, 0x66), 15, juce::Point<int>(5, 5));
    light = juce::DropShadow(juce::Colour::fromRGBA(0x40, 0x60, 0x80, 0x20), 15, juce::Point<int>(-5, -5));
}

TherewillnotbebloodAudioProcessorEditor::~TherewillnotbebloodAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TherewillnotbebloodAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setColour(juce::Colour::fromRGB(0x18, 0x20, 0x2A));
    g.fillAll();

    // Draw coconut plugins image
    g.drawImage(coconutPluginsImage, imageRect.toFloat());

    // Draw title
    g.setColour(juce::Colours::white);
    g.setFont(typeface);
    g.setFont(28.0f);
    g.drawText("There will not be blood", getLocalBounds().getCentreX() - 100, 25, 200, 20, juce::Justification::centred);

    // Draw shadow and light for the components
    shadow.drawForRectangle(g, levelMeter.getBounds());
    light.drawForRectangle(g, levelMeter.getBounds());
    shadow.drawForRectangle(g, spectrumAnalyzer.getBounds());
    light.drawForRectangle(g, spectrumAnalyzer.getBounds());

    bypassButton.repaint();
}

void TherewillnotbebloodAudioProcessorEditor::resized()
{
    // Logo
    imageRect.setX(getWidth() - 70);
    imageRect.setY(10);
    imageRect.setWidth(50);
    imageRect.setHeight(50);

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

void TherewillnotbebloodAudioProcessorEditor::timerCallback()
{
	// Get the rms value from the audio processor
    float dryRmsValue = juce::jlimit(levelMeter.mindB, levelMeter.maxdB, audioProcessor.getRmsValue(true));
    float wetRmsValue = juce::jlimit(levelMeter.mindB, levelMeter.maxdB, audioProcessor.getRmsValue(false));

	// add the rms value to the level meter
	levelMeter.addRmsValues(dryRmsValue, wetRmsValue);

    // Perform fft
    window.multiplyWithWindowingTable(audioProcessor.dryFftData, audioProcessor.fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(audioProcessor.dryFftData);
    window.multiplyWithWindowingTable(audioProcessor.wetFftData, audioProcessor.fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(audioProcessor.wetFftData);
    
    // Update the spectrum analyzer
    spectrumAnalyzer.updateSpectra(audioProcessor.dryFftData, audioProcessor.wetFftData, audioProcessor.fftSize);
    audioProcessor.nextDryFFTBlockReady = false;
    audioProcessor.nextWetFFTBlockReady = false;

    repaint();
}

void TherewillnotbebloodAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &thresholdSlider) {
        levelMeter.setThreshold(thresholdSlider.getValue());
        audioProcessor.setThreshold(thresholdSlider.getValue());
    }
    else if (slider == &cutoffSlider) {
		spectrumAnalyzer.setCutoff(cutoffSlider.getValue());
        audioProcessor.setCutoff(cutoffSlider.getValue());
	}
}