#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LevelMeter.h"
#include "SpectrumAnalyzer.h"
#include "CustomLookAndFeel.h"

class Editor final : public AudioProcessorEditor, public Timer, public Slider::Listener
{
public:
    explicit Editor (Processor&);
    ~Editor() override;

    void paint (Graphics&) override;
    void resized() override;

    void timerCallback() override;
    void sliderValueChanged(Slider*) override;

private:
    Processor& processorRef;

    // Title
    Typeface::Ptr typeface = Typeface::createSystemTypefaceFor(BinaryData::kraut____typefuck11_ttf, BinaryData::kraut____typefuck11_ttfSize);

    // FFT
    const static int interpolatedSize = 16000;
    dsp::FFT forwardFFT;
    dsp::WindowingFunction<float> window;
    LagrangeInterpolator dryLagrangeInterpolator;
    LagrangeInterpolator wetLagrangeInterpolator;
    float dryInterpolatedFftData[interpolatedSize];
    float wetInterpolatedFftData[interpolatedSize];

    // Components
    LevelMeter levelMeter;
    SpectrumAnalyzer spectrumAnalyzer;

    // Shadow and light
    DropShadow shadow;
    DropShadow light;

    // Sliders and buttons
    Slider thresholdSlider;
    Slider cutoffSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> thresholdSliderAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> cutoffSliderAttachment;
    ToggleButton bypassButton;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> bypassButtonAttachment;

    // LookAndFeel
    CustomLookAndFeel lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
};
