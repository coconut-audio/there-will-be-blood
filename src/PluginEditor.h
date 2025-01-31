#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LevelMeter.h"
#include "SpectrumAnalyzer.h"
#include "CustomLookAndFeel.h"

#define INTERPOLATIONSIZE 16000

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor, public juce::Timer, public juce::Slider::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    void timerCallback() override;
    void sliderValueChanged(juce::Slider*) override;

private:
    AudioPluginAudioProcessor& processorRef;

    // Shadow and light
    juce::DropShadow shadow;
    juce::DropShadow light;

    // Logo
    juce::Image coconutPluginsImage = juce::ImageCache::getFromMemory(BinaryData::coconut_audio_png, BinaryData::coconut_audio_pngSize);
    juce::Rectangle<int> imageRect;

    // Title
    juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::kraut____typefuck11_ttf, BinaryData::kraut____typefuck11_ttfSize);

    // Components
    LevelMeter levelMeter;
    SpectrumAnalyzer spectrumAnalyzer;

    // FFT
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    juce::LagrangeInterpolator dryLagrangeInterpolator;
    juce::LagrangeInterpolator wetLagrangeInterpolator;
    float dryInterpolatedFftData[INTERPOLATIONSIZE];
    float wetInterpolatedFftData[INTERPOLATIONSIZE];
    float ratio = processorRef.fftSize / (float)INTERPOLATIONSIZE;

    // Sliders and buttons
    juce::Slider thresholdSlider;
    juce::Slider cutoffSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffSliderAttachment;
    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassButtonAttachment;

    // LookAndFeel
    CustomLookAndFeel lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
