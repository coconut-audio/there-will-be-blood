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
    startTimerHz(frameRate);

    addAndMakeVisible(levelMeter);
    addAndMakeVisible(spectrumAnalyzer);

    thresholdSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "threshold", thresholdSlider);
    thresholdSlider.setSliderStyle(juce::Slider::LinearVertical);
    thresholdSlider.setRange(levelMeter.mindB, levelMeter.maxdB, 0.01f);
    thresholdSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    thresholdSlider.addListener(this);
    thresholdSlider.setLookAndFeel(&verticalSliderLookAndFeel);
    addAndMakeVisible(&thresholdSlider);
    thresholdSlider.setValue(audioProcessor.apvts.getRawParameterValue("threshold")->load());
    levelMeter.setThreshold(thresholdSlider.getValue());

    cutoffSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "cutoff", cutoffSlider);
    cutoffSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    cutoffSlider.setRange(spectrumAnalyzer.minHz, spectrumAnalyzer.maxHz, 0.1f);
    cutoffSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    cutoffSlider.addListener(this);
    cutoffSlider.setLookAndFeel(&horizontalSliderLookAndFeel);
    addAndMakeVisible(&cutoffSlider);
    cutoffSlider.setValue(audioProcessor.apvts.getRawParameterValue("cutoff")->load());
    spectrumAnalyzer.setCutoff(cutoffSlider.getValue());

    bypassButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "bypass", bypassButton);
    bypassButton.setLookAndFeel(&toggleButtonLookAndFeel);
    bypassButton.onClick = [this]() {
		levelMeter.setBypass(bypassButton.getToggleState());
        spectrumAnalyzer.setBypass(bypassButton.getToggleState());
	};
    addAndMakeVisible(&bypassButton);
    bypassButton.setToggleState(audioProcessor.apvts.getRawParameterValue("bypass")->load(), juce::NotificationType::dontSendNotification);

    spectrumAnalyzer.sampleRate = audioProcessor.getSampleRate();
    spectrumAnalyzer.fftSize = audioProcessor.fftSize;

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
    shadow.drawForRectangle(g, titleRect);
    light.drawForRectangle(g, titleRect);
    g.setColour(juce::Colour::fromRGB(0x18, 0x20, 0x2A));
    g.fillRoundedRectangle(titleRect.toFloat(), 10);
    g.setColour(juce::Colours::white);
    g.setGradientFill(titleGradient);
    g.setFont(16.0f);
    g.drawText("There will not be blood", getLocalBounds().getCentreX() - 100, 25, 200, 20, juce::Justification::centred);

    shadow.drawForRectangle(g, levelMeter.getBounds());
    light.drawForRectangle(g, levelMeter.getBounds());
    
    shadow.drawForRectangle(g, spectrumAnalyzer.getBounds());
    light.drawForRectangle(g, spectrumAnalyzer.getBounds());

    bypassButton.repaint();
}

void TherewillnotbebloodAudioProcessorEditor::resized()
{
    imageRect.setX(getWidth() - 70);
    imageRect.setY(10);
    imageRect.setWidth(50);
    imageRect.setHeight(50);

    titleRect.setBounds(getLocalBounds().getCentreX() - 110, 20, 220, 30);
    float x = (titleRect.getWidth() - titleRect.getCentreX()) * cosf(phase);
    float y = (titleRect.getHeight() - titleRect.getCentreY()) * sinf(phase);
    titleGradient = juce::ColourGradient(juce::Colour::fromRGB(0x0D, 0x92, 0xF4), titleRect.getCentreX() + x, titleRect.getCentreY() + y, juce::Colour::fromRGB(0xC3, 0x0E, 0x59), titleRect.getCentreX() - x, titleRect.getCentreY() - y, false);

    juce::Rectangle<int> levelMeterBounds(getLocalBounds());
    levelMeterBounds.setY(60);
    levelMeterBounds.setHeight(200);
    levelMeterBounds = levelMeterBounds.reduced(20);
    levelMeter.setBounds(levelMeterBounds);

    thresholdSlider.setBounds(levelMeterBounds.getX() + 60, levelMeterBounds.getY() + 20, 15, levelMeterBounds.getHeight() - 40);

    juce::Rectangle<int> spectrumAnalyserBounds = getLocalBounds();
    spectrumAnalyserBounds.setY(levelMeterBounds.getBottom() + 10);
    spectrumAnalyserBounds.setHeight(200);
    spectrumAnalyserBounds = spectrumAnalyserBounds.reduced(20);
    spectrumAnalyzer.setBounds(spectrumAnalyserBounds);

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

    // Gradient animation
    float x = (titleRect.getWidth() - titleRect.getCentreX()) * cosf(phase);
    float y = -(titleRect.getWidth() - titleRect.getCentreX()) * sinf(phase);
    titleGradient = juce::ColourGradient(juce::Colour::fromRGB(0x0D, 0x92, 0xF4), titleRect.getCentreX() + x, titleRect.getCentreY() + y, juce::Colour::fromRGB(0xC3, 0x0E, 0x59), titleRect.getCentreX() - x, titleRect.getCentreY() - y, false);
    
    phase += 2.0f * juce::float_Pi / (10.0f * frameRate);
    phase = fmodf(phase, 2.0f * juce::float_Pi);
    toggleButtonLookAndFeel.phase = phase;

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