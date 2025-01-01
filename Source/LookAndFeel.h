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

class ToggleButtonLookAndFeel : public juce::LookAndFeel_V4 {
public:
	
	ToggleButtonLookAndFeel() = default;

	void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
		juce::DropShadow shadow(juce::Colour::fromRGBA(0x00, 0x00, 0x00, 0x66), 15, juce::Point<int>(5, 5));
		juce::DropShadow light(juce::Colour::fromRGBA(0x40, 0x60, 0x80, 0x20), 15, juce::Point<int>(-5, -5));

		juce::Rectangle<float> buttonRect = button.getLocalBounds().toFloat();
		buttonRect = buttonRect.reduced(10.0f);

		if (!button.getToggleState()) {
			float x = (buttonRect.getWidth() - buttonRect.getCentreX() / 0.5f) * cosf(phase);
			float y = -(buttonRect.getHeight() - buttonRect.getCentreY() / 0.5f) * sinf(phase);
			buttonGradient = juce::ColourGradient(juce::Colour::fromRGB(0x0D, 0x92, 0xF4), buttonRect.getCentreX() + x, buttonRect.getCentreY() + y, juce::Colour::fromRGB(0xC3, 0x0E, 0x59), buttonRect.getCentreX() - x, buttonRect.getCentreY() - y, false);
		}
		else {
			// Draw shadow and light
			shadow.drawForRectangle(g, buttonRect.toNearestInt());
			light.drawForRectangle(g, buttonRect.toNearestInt());

			buttonGradient = juce::ColourGradient(juce::Colour::fromRGBA(0x55, 0x55, 0x55, 0x88), buttonRect.getX(), buttonRect.getY(), juce::Colour::fromRGBA(0x55, 0x55, 0x55, 0x88), buttonRect.getX(), buttonRect.getBottom(), false);
		}

		// Draw background
		g.setGradientFill(buttonGradient);
		g.fillRoundedRectangle(buttonRect.toFloat(), 5.0f);
	}

	float phase = 0.0f;

private:
	juce::ColourGradient buttonGradient;
};