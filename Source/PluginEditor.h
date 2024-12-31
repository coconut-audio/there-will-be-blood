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

    juce::DropShadow shadow;
    juce::DropShadow light;

    juce::Image coconutPluginsImage = juce::ImageCache::getFromMemory(BinaryData::coconut_audio_png, BinaryData::coconut_audio_pngSize);
    juce::Rectangle<int> imageRect;

    juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::AdventPro_ExpandedExtraBold_ttf, BinaryData::AdventPro_ExpandedExtraBold_ttfSize);
    juce::Rectangle<int> titleRect;
    juce::ColourGradient titleGradient;

    LevelMeter levelMeter;
    SpectrumAnalyzer spectrumAnalyzer;

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    juce::Slider thresholdSlider;
    juce::Slider cutoffSlider;

    VerticalSliderLookAndFeel verticalSliderLookAndFeel;
    HorizontalSliderLookAndFeel horizontalSliderLookAndFeel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffSliderAttachment;

    int frameRate = 30;
    float phase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TherewillnotbebloodAudioProcessorEditor)
};