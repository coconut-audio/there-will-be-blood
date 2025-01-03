/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LevelMeter.h"
#include "SpectrumAnalyzer.h"
#include "LookAndFeel.h"

//==============================================================================
/**
*/
class TherewillnotbebloodAudioProcessorEditor  : public juce::AudioProcessorEditor,
    public juce::Timer, public juce::Slider::Listener
{
public:
    TherewillnotbebloodAudioProcessorEditor (TherewillnotbebloodAudioProcessor&);
    ~TherewillnotbebloodAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    void timerCallback() override;
    void sliderValueChanged(juce::Slider*) override;

private:
    TherewillnotbebloodAudioProcessor& audioProcessor;

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

    // Sliders and buttons
    juce::Slider thresholdSlider;
    juce::Slider cutoffSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffSliderAttachment;
    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassButtonAttachment;

    // LookAndFeel
    LookAndFeel lookAndFeel;

    // Variables
    int frequency = 30;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TherewillnotbebloodAudioProcessorEditor)
};