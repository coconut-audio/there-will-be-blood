#include "LevelMeter.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

Editor::Editor (Processor& p)
    : AudioProcessorEditor (&p)
    , processorRef (p)
    , forwardFFT(processorRef.fftOrder)
    , window(1 << processorRef.fftOrder, dsp::WindowingFunction<float>::hann)
    , levelMeter(p)
    , spectrumAnalyzer(p)
    , shadow(Colour::fromRGBA(0x00, 0x00, 0x00, 0x66), 15, Point<int>(5, 5))
    , light(Colour::fromRGBA(0x48, 0x47, 0x4D, 0x20), 15, Point<int>(-5, -5))
{
    setSize (600, 450);
    startTimerHz(30);

    addAndMakeVisible(levelMeter);
    addAndMakeVisible(spectrumAnalyzer);

    // Threshold slider
    thresholdSliderAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "threshold", thresholdSlider);
    thresholdSlider.setSliderStyle(Slider::LinearVertical);
    thresholdSlider.setRange(levelMeter.mindB, levelMeter.maxdB, 0.01f);
    thresholdSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    thresholdSlider.addListener(this);
    thresholdSlider.setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(&thresholdSlider);
    thresholdSlider.setValue(processorRef.apvts.getRawParameterValue("threshold")->load());

    // Cutoff slider
    cutoffSliderAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "cutoff", cutoffSlider);
    cutoffSlider.setSliderStyle(Slider::LinearHorizontal);
    cutoffSlider.setRange(spectrumAnalyzer.minHz, spectrumAnalyzer.maxHz, 0.1f);
    cutoffSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    cutoffSlider.addListener(this);
    cutoffSlider.setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(&cutoffSlider);
    cutoffSlider.setValue(processorRef.apvts.getRawParameterValue("cutoff")->load());

    // Bypass button
    bypassButtonAttachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(processorRef.apvts, "bypass", bypassButton);
    bypassButton.setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(&bypassButton);
    bool bypass = processorRef.apvts.getRawParameterValue("bypass")->load() > 0.5f;
    bypassButton.setToggleState(bypass, NotificationType::dontSendNotification);
}

Editor::~Editor()
{
    stopTimer();
}

void Editor::paint (Graphics& g)
{
    g.setColour(Colour(0x18, 0x17, 0x1D));
    g.fillAll();

    // Draw title
    g.setColour(Colour(0xF6, 0xEF, 0xDE));
    g.setFont(FontOptions(typeface).withHeight(42.0f));
    g.drawText("There will be blood", getLocalBounds().getCentreX() - 150, 10, 300, 50, juce::Justification::centred);
    g.setOpacity(1.0f);

    // Draw shadow and light for the components
    shadow.drawForRectangle(g, levelMeter.getBounds());
    light.drawForRectangle(g, levelMeter.getBounds());
    shadow.drawForRectangle(g, spectrumAnalyzer.getBounds());
    light.drawForRectangle(g, spectrumAnalyzer.getBounds());

    bypassButton.repaint();
}

void Editor::resized()
{
    // Level meter
    Rectangle<int> levelMeterBounds(getLocalBounds());
    levelMeterBounds.setY(60);
    levelMeterBounds.setHeight(200);
    levelMeterBounds = levelMeterBounds.reduced(20);
    levelMeter.setBounds(levelMeterBounds);

    // Spectrum analyzer
    Rectangle<int> spectrumAnalyserBounds = getLocalBounds();
    spectrumAnalyserBounds.setY(levelMeterBounds.getBottom() + 10);
    spectrumAnalyserBounds.setHeight(200);
    spectrumAnalyserBounds = spectrumAnalyserBounds.reduced(20);
    spectrumAnalyzer.setBounds(spectrumAnalyserBounds);

    // Sliders and buttons
    thresholdSlider.setBounds(levelMeterBounds.getX() + 60, levelMeterBounds.getY() + 20, 15, levelMeterBounds.getHeight() - 40);
    cutoffSlider.setBounds(spectrumAnalyserBounds.getCentreX() - spectrumAnalyserBounds.getWidth() / 3.0f, spectrumAnalyserBounds.getBottom() - 60, 2.0f * spectrumAnalyserBounds.getWidth() / 3.0f, 15);
    bypassButton.setBounds(20, 15, 40, 40);
}

void Editor::timerCallback()
{
	// Get the rms value from the audio processor
    float dryRmsValue = jlimit(levelMeter.mindB, levelMeter.maxdB, processorRef.getRmsValue(true));
    float wetRmsValue = jlimit(levelMeter.mindB, levelMeter.maxdB, processorRef.getRmsValue(false));

	// add the rms value to the level meter
	levelMeter.fillRmsValues(dryRmsValue, wetRmsValue);

    // Perform fft
    window.multiplyWithWindowingTable(processorRef.dryFftData, processorRef.fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(processorRef.dryFftData);
    window.multiplyWithWindowingTable(processorRef.wetFftData, processorRef.fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(processorRef.wetFftData);

    // interpolate fft data
    dryLagrangeInterpolator.process((float)processorRef.fftSize / (float)interpolatedSize, processorRef.dryFftData, dryInterpolatedFftData, interpolatedSize);
    wetLagrangeInterpolator.process((float)processorRef.fftSize / (float)interpolatedSize, processorRef.wetFftData, wetInterpolatedFftData, interpolatedSize);

    // Update the spectrum analyzer
    spectrumAnalyzer.updateSpectra(dryInterpolatedFftData, wetInterpolatedFftData, (float)interpolatedSize);
    processorRef.nextDryFFTBlockReady = false;
    processorRef.nextWetFFTBlockReady = false;

    repaint();
}

void Editor::sliderValueChanged(Slider* slider)
{
    if (slider == &thresholdSlider) {
        processorRef.setCompressorThreshold(processorRef.apvts.getRawParameterValue("threshold")->load());
    } else if (slider == &cutoffSlider) {
        processorRef.setFilterCutoff(processorRef.apvts.getRawParameterValue("cutoff")->load());
	}
}
