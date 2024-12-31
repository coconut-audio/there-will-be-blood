/*
  ==============================================================================

    ThresholdSlider.h
    Created: 28 Dec 2024 11:15:23pm
    Author:  vivek

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class VerticalSliderLookAndFeel : public juce::LookAndFeel_V4 {
public:

    VerticalSliderLookAndFeel() = default;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) override {
        float thumbRadius = 5.0f;
        juce::Rectangle<int> backgroundRect(x + width / 2 - 5, y + 2, 10, height - 4);
        juce::Rectangle<int> sliderRect = backgroundRect;
        sliderRect.setTop(sliderPos);

        // Draw shadow
        juce::DropShadow shadow(juce::Colour::fromRGBA(0x00, 0x00, 0x00, 0x20), 5, juce::Point<int>(2, 2));
        shadow.drawForRectangle(g, backgroundRect);

        // Draw background
        g.setColour(juce::Colour::fromRGBA(0x32, 0x3E, 0x49, 0xAA));
        g.fillRoundedRectangle(backgroundRect.toFloat(), 5.0f);

        // Draw slider
        g.setColour(juce::Colour::fromRGBA(0xC3, 0x0E, 0x59, 0xAA));
        g.fillRoundedRectangle(sliderRect.toFloat(), 5.0f);

        // Draw thumb
		g.setColour(juce::Colour::fromRGB(0xFF, 0xFF, 0xFF));
		g.fillEllipse(x + width / 2 - thumbRadius, sliderPos - thumbRadius, 2.0f * thumbRadius, 2.0f * thumbRadius);
	}
};

class HorizontalSliderLookAndFeel : public juce::LookAndFeel_V4 {
    public:

	HorizontalSliderLookAndFeel() = default;

	void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) override {
		float thumbRadius = 5.0f;
		juce::Rectangle<int> backgroundRect(x + 2, y + height / 2 - 4, width - 4, 8);
		juce::Rectangle<int> sliderRect = backgroundRect;
		sliderRect.setLeft(sliderPos);

		// Draw shadow
		juce::DropShadow shadow(juce::Colour::fromRGBA(0x00, 0x00, 0x00, 0x20), 5, juce::Point<int>(2, 2));
		shadow.drawForRectangle(g, backgroundRect);

		// Draw background
		g.setColour(juce::Colour::fromRGBA(0x32, 0x3E, 0x49, 0xAA));
		g.fillRoundedRectangle(backgroundRect.toFloat(), 5.0f);

		// Draw slider
		g.setColour(juce::Colour::fromRGBA(0xC3, 0x0E, 0x59, 0xAA));
		g.fillRoundedRectangle(sliderRect.toFloat(), 5.0f);

		// Draw thumb
		g.setColour(juce::Colour::fromRGB(0xFF, 0xFF, 0xFF));
		g.fillEllipse(sliderPos - thumbRadius, y + height / 2 - thumbRadius, 2.0f * thumbRadius, 2.0f * thumbRadius);
	}
};