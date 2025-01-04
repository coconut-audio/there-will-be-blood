/*
  ==============================================================================

    ThresholdSlider.h
    Created: 28 Dec 2024 11:15:23pm
    Author:  vivek

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4 
{
public:
    CustomLookAndFeel() = default;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) override {
		float thumbRadius = 5.0f;
		juce::Rectangle<float> backgroundRect;
		juce::Rectangle<float> sliderRect;
		juce::Rectangle<float> thumbRect;

		if (slider.getSliderStyle() == juce::Slider::SliderStyle::LinearVertical) {
			backgroundRect = juce::Rectangle<float>(x + width / 2 - 5, y + 2, 10, height - 4);
			sliderRect = backgroundRect;
			sliderRect.setTop(sliderPos);
			thumbRect = juce::Rectangle<float>(x + width / 2 - thumbRadius, sliderPos - thumbRadius, 2.0f * thumbRadius, 2.0f * thumbRadius);
		}
		else if (slider.getSliderStyle() == juce::Slider::SliderStyle::LinearHorizontal) {
			backgroundRect = juce::Rectangle<float>(x + 2, y + height / 2 - 4, width - 4, 8);
			sliderRect = backgroundRect;
			sliderRect.setLeft(sliderPos);
			thumbRect = juce::Rectangle<float>(sliderPos - thumbRadius, y + height / 2 - thumbRadius, 2.0f * thumbRadius, 2.0f * thumbRadius);
		}
		
        // Draw shadow
		juce::DropShadow shadow(juce::Colour::fromRGBA(0x00, 0x00, 0x00, 0x44), 15, juce::Point<int>(5, 5));
        shadow.drawForRectangle(g, backgroundRect.toNearestInt());

        // Draw background
        g.setColour(juce::Colour::fromRGBA(0x32, 0x3E, 0x49, 0xAA));
        g.fillRoundedRectangle(backgroundRect.toFloat(), 5.0f);

        // Draw slider
		g.setColour(juce::Colour::fromRGB(0x48, 0x47, 0x4D));
        g.fillRoundedRectangle(sliderRect.toFloat(), 5.0f);

        // Draw thumb
		g.setColour(juce::Colour::fromRGB(0xFF, 0xFF, 0xFF));
		g.fillEllipse(thumbRect.toFloat());
	}

	void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
		juce::Rectangle<float> buttonRect = button.getLocalBounds().toFloat();
		buttonRect = buttonRect.reduced(12.0f);
		juce::Path buttonPath;
		buttonPath.addEllipse(buttonRect);

		// Draw shadow and light
		juce::DropShadow shadow(juce::Colour::fromRGBA(0x00, 0x00, 0x00, 0x66), 15, juce::Point<int>(5, 5));
		juce::DropShadow light(juce::Colour::fromRGBA(0x48, 0x47, 0x4D, 0x20), 15, juce::Point<int>(-5, -5));
		juce::DropShadow glow(juce::Colour::fromRGBA(0xF6, 0xEF, 0xDE, 0x44), 15, juce::Point<int>(0, 0));
		shadow.drawForPath(g, buttonPath);
		light.drawForPath(g, buttonPath);

		// Draw button
		if (!button.getToggleState()) {
			g.setColour(juce::Colour::fromRGB(0xF6, 0xEF, 0xDE));
			g.fillPath(buttonPath);
			glow.drawForPath(g, buttonPath);
		}
		else {
			// Draw button
			g.setColour(juce::Colour::fromRGBA(0x38, 0x37, 0x3D, 0x88));
			g.fillPath(buttonPath);
		}
	}
};