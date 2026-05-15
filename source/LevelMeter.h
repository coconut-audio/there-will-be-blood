#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class LevelMeter  : public Component
{
public:
    LevelMeter(Processor&);
    ~LevelMeter() override;

    void paint (Graphics&) override;
    void resized() override;

    void fillRmsValues(float, float);

    constexpr static float mindB = -60.0f;
    constexpr static float maxdB = 36.0f;
    constexpr static int bufferSize = 256;


private:
    Processor& processorRef;

    Rectangle<float> backgroundRect;

    ColourGradient levelGradient;
    ColourGradient radialGradient;
    DropShadow shadow;
    DropShadow light;

    std::vector<float> dryRmsValues;
    std::vector<float> wetRmsValues;

    constexpr static float cornerSize = 10.0f;
    constexpr static float strokeThickness = 2.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
