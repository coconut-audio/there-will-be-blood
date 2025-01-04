/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TherewillnotbebloodAudioProcessorEditor::TherewillnotbebloodAudioProcessorEditor (TherewillnotbebloodAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), forwardFFT(audioProcessor.fftOrder), window(1 << audioProcessor.fftOrder, juce::dsp::WindowingFunction<float>::hann)
{
    setSize (600, 450);
    startTimerHz(30);

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
    light = juce::DropShadow(juce::Colour::fromRGBA(0x48, 0x47, 0x4D, 0x20), 15, juce::Point<int>(-5, -5));
}

TherewillnotbebloodAudioProcessorEditor::~TherewillnotbebloodAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TherewillnotbebloodAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setColour(juce::Colour::fromRGB(0x18, 0x17, 0x1D));
    g.fillAll();

    // Draw title
    g.setColour(juce::Colour::fromRGB(0xF6, 0xEF, 0xDE));
    g.setFont(typeface);
    g.setFont(42.0f);
    g.drawText("There will not be blood", getLocalBounds().getCentreX() - 150, 10, 300, 50, juce::Justification::centred);

    // fill texture
    g.setOpacity(0.05f);
    g.drawImageWithin(textureImage, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);

    g.setOpacity(1.0f);
    // Draw coconut plugins image
    g.drawImage(coconutPluginsImage, imageRect.toFloat());

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
    imageRect.setY(15);
    imageRect.setWidth(40);
    imageRect.setHeight(40);

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

    // interpolate fft data
    dryLagrangeInterpolator.process(ratio, audioProcessor.dryFftData, dryInterpolatedFftData, INTERPOLATIONSIZE);
    wetLagrangeInterpolator.process(ratio, audioProcessor.wetFftData, wetInterpolatedFftData, INTERPOLATIONSIZE);
    
    // Update the spectrum analyzer
    spectrumAnalyzer.updateSpectra(dryInterpolatedFftData, wetInterpolatedFftData, INTERPOLATIONSIZE);
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