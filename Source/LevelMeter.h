/*
  ==============================================================================

    LevelMeter.h
    Created: 28 Dec 2024 9:22:54pm
    Author:  vivek

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

//==============================================================================
/*
*/
class LevelMeter  : public juce::Component
{
public:
    LevelMeter();
    ~LevelMeter() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void addRmsValues(float, float);
    void setThreshold(float);
    void setBypass(bool);

    float mindB = -60.0f;
    float maxdB = 36.0f;

private:
    juce::Rectangle<float> backgroundRect;

    juce::ColourGradient waveformGradient;
    juce::ColourGradient radialGradient;
    juce::DropShadow shadow;
    juce::DropShadow light;

    std::vector<float> dryRmsValues;
    std::vector<float> wetRmsValues;
    int bufferSize = 256;

    float cornerSize = 10.0f;
    float strokeThickness = 2.0f;

    float threshold;
    float bypass = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
