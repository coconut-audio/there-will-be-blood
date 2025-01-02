/*
  ==============================================================================

    ThresholdSlider.h
    Created: 28 Dec 2024 11:15:23pm
    Author:  vivek

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class LookAndFeel : public juce::LookAndFeel_V4 
{
public:
    LookAndFeel() = default;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) override {
		float thumbRadius = 5.0f;
		juce::Rectangle<int> backgroundRect;
		juce::Rectangle<int> sliderRect;
		juce::Rectangle<int> thumbRect;

		if (slider.getSliderStyle() == juce::Slider::SliderStyle::LinearVertical) {
			backgroundRect = juce::Rectangle<int>(x + width / 2 - 5, y + 2, 10, height - 4);
			sliderRect = backgroundRect;
			sliderRect.setTop(sliderPos);
			thumbRect = juce::Rectangle<int>(x + width / 2 - thumbRadius, sliderPos - thumbRadius, 2.0f * thumbRadius, 2.0f * thumbRadius);
		}
		else if (slider.getSliderStyle() == juce::Slider::SliderStyle::LinearHorizontal) {
			backgroundRect = juce::Rectangle<int>(x + 2, y + height / 2 - 4, width - 4, 8);
			sliderRect = backgroundRect;
			sliderRect.setLeft(sliderPos);
			thumbRect = juce::Rectangle<int>(sliderPos - thumbRadius, y + height / 2 - thumbRadius, 2.0f * thumbRadius, 2.0f * thumbRadius);
		}
		
        // Draw shadow
		juce::DropShadow shadow(juce::Colour::fromRGBA(0x00, 0x00, 0x00, 0x44), 15, juce::Point<int>(5, 5));
        shadow.drawForRectangle(g, backgroundRect);

        // Draw background
        g.setColour(juce::Colour::fromRGBA(0x32, 0x3E, 0x49, 0xAA));
        g.fillRoundedRectangle(backgroundRect.toFloat(), 5.0f);

        // Draw slider
        g.setColour(juce::Colour::fromRGBA(0xC3, 0x0E, 0x59, 0xAA));
        g.fillRoundedRectangle(sliderRect.toFloat(), 5.0f);

        // Draw thumb
		g.setColour(juce::Colour::fromRGB(0xFF, 0xFF, 0xFF));
		g.fillEllipse(thumbRect.toFloat());
	}

	void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
		juce::Rectangle<float> buttonRect = button.getLocalBounds().toFloat();
		buttonRect = buttonRect.reduced(10.0f);
		juce::Path buttonPath;
		buttonPath.addEllipse(buttonRect);

		if (!button.getToggleState()) {
			// Draw shadow and light
			juce::DropShadow shadow(juce::Colour::fromRGBA(0x00, 0x00, 0x00, 0x44), 10, juce::Point<int>(3, 3));
			juce::DropShadow light(juce::Colour::fromRGBA(0x40, 0x60, 0x80, 0x20), 10, juce::Point<int>(-3, -3));
			shadow.drawForPath(g, buttonPath);
			light.drawForPath(g, buttonPath);

			// Draw button
			float x = (buttonRect.getWidth() - buttonRect.getCentreX() / 0.5f) * cosf(phase);
			float y = -(buttonRect.getHeight() - buttonRect.getCentreY() / 0.5f) * sinf(phase);
			buttonGradient = juce::ColourGradient(juce::Colour::fromRGB(0x0D, 0x92, 0xF4), buttonRect.getCentreX() + x, buttonRect.getCentreY() + y, juce::Colour::fromRGB(0xC3, 0x0E, 0x59), buttonRect.getCentreX() - x, buttonRect.getCentreY() - y, false);
			g.setGradientFill(buttonGradient);
			g.fillPath(buttonPath);
		}
		else {
			// Draw shadow and light
			juce::DropShadow shadow(juce::Colour::fromRGBA(0x00, 0x00, 0x00, 0x44), 15, juce::Point<int>(5, 5));
			juce::DropShadow light(juce::Colour::fromRGBA(0x40, 0x60, 0x80, 0x20), 15, juce::Point<int>(-5, -5));
			shadow.drawForPath(g, buttonPath);
			light.drawForPath(g, buttonPath);

			g.setColour(juce::Colour::fromRGBA(0x42, 0x4E, 0x59, 0x88));
			g.fillPath(buttonPath);
		}
	}

	float phase = 0.0f;

private:
	juce::ColourGradient buttonGradient;
};