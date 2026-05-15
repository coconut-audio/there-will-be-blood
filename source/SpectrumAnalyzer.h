#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SpectrumAnalyzer  : public Component
{
public:
    SpectrumAnalyzer(Processor&);
    ~SpectrumAnalyzer() override;

    void paint (Graphics&) override;
    void resized() override;

    void updateSpectra(float*, float*, float);

    constexpr static float mindB = -60.0f;
    constexpr static float maxdB = 36.0f;
    constexpr static float minHz = 20.0f;
    constexpr static float maxHz = 20000.0f;


private:
    Processor& processorRef;

    Rectangle<float> backgroundRect;

    ColourGradient spectrumGradient;
    DropShadow shadow;
    DropShadow light;

    constexpr static int scopeSize = 512;
    float dryScopeData[scopeSize];
    float wetScopeData[scopeSize];

    std::vector<int> frequencies = { 80, 250, 500, 1000, 2000, 4000, 10000 };
    constexpr static float cornerSize = 10.0f;
    constexpr static float strokeThickness = 2.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyzer)
};
