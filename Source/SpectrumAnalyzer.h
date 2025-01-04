/*
  ==============================================================================

    SpectrumAnalyzer.h
    Created: 28 Dec 2024 9:29:46pm
    Author:  vivek

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define SCOPESIZE 512

//==============================================================================
/*
*/
class SpectrumAnalyzer  : public juce::Component
{
public:
    SpectrumAnalyzer();
    ~SpectrumAnalyzer() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void SpectrumAnalyzer::updateSpectra(float*, float*, float);
    void setCutoff(float);
    void setBypass(bool);

    int sampleRate;
    int fftSize;

    float mindB = -60.0f;
    float maxdB = 36.0f;

    float minHz = 20.0f;
    float maxHz = 20000.0f;

private:
    juce::Rectangle<float> backgroundRect;

    juce::ColourGradient spectrumGradient;
    juce::DropShadow shadow;
    juce::DropShadow light;

    float dryScopeData[SCOPESIZE];
    float wetScopeData[SCOPESIZE];

    std::vector<int> frequencies = { 80, 250, 500, 1000, 2000, 4000, 10000 };

    float cornerSize = 10.0f;
    float strokeThickness = 2.0f;

    float cutoff;
    float bypass = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyzer)
};
