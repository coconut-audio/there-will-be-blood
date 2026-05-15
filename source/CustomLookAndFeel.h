#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public LookAndFeel_V4
{
public:
    CustomLookAndFeel() = default;

    void drawLinearSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float, float /*maxSliderPos*/, const Slider::SliderStyle /*style*/, Slider& slider) override {
		float thumbRadius = 5.0f;
		Rectangle<float> backgroundRect;
		Rectangle<float> sliderRect;
		Rectangle<float> thumbRect;

		if (slider.getSliderStyle() == Slider::SliderStyle::LinearVertical) {
			backgroundRect = Rectangle<float>(x + width / 2 - 5, y + 2, 10, height - 4);
			sliderRect = backgroundRect;
			sliderRect.setTop(sliderPos);
			thumbRect = Rectangle<float>(x + width / 2 - thumbRadius, sliderPos - thumbRadius, 2.0f * thumbRadius, 2.0f * thumbRadius);
		}
		else if (slider.getSliderStyle() == Slider::SliderStyle::LinearHorizontal) {
			backgroundRect = Rectangle<float>(x + 2, y + height / 2 - 4, width - 4, 8);
			sliderRect = backgroundRect;
			sliderRect.setLeft(sliderPos);
			thumbRect = Rectangle<float>(sliderPos - thumbRadius, y + height / 2 - thumbRadius, 2.0f * thumbRadius, 2.0f * thumbRadius);
		}

        // Draw shadow
		DropShadow shadow(Colour::fromRGBA(0x00, 0x00, 0x00, 0x44), 15, Point<int>(5, 5));
        shadow.drawForRectangle(g, backgroundRect.toNearestInt());

        // Draw background
        g.setColour(Colour::fromRGBA(0x32, 0x3E, 0x49, 0xAA));
        g.fillRoundedRectangle(backgroundRect.toFloat(), 5.0f);

        // Draw slider
		g.setColour(Colour::fromRGB(0x48, 0x47, 0x4D));
        g.fillRoundedRectangle(sliderRect.toFloat(), 5.0f);

        // Draw thumb
		g.setColour(Colour::fromRGB(0xFF, 0xFF, 0xFF));
		g.fillEllipse(thumbRect.toFloat());
	}

	void drawToggleButton(Graphics& g, ToggleButton& button, bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/) override {
		Rectangle<float> buttonRect = button.getLocalBounds().toFloat();
		buttonRect = buttonRect.reduced(12.0f);
		Path buttonPath;
		buttonPath.addEllipse(buttonRect);

		// Draw shadow and light
		DropShadow shadow(Colour::fromRGBA(0x00, 0x00, 0x00, 0x66), 15, Point<int>(5, 5));
		DropShadow light(Colour::fromRGBA(0x48, 0x47, 0x4D, 0x20), 15, Point<int>(-5, -5));
		DropShadow glow(Colour::fromRGBA(0xF6, 0xEF, 0xDE, 0x44), 15, Point<int>(0, 0));
		shadow.drawForPath(g, buttonPath);
		light.drawForPath(g, buttonPath);

		// Draw button
		if (!button.getToggleState()) {
			g.setColour(Colour(0xF6, 0xEF, 0xDE));
			g.fillPath(buttonPath);
			glow.drawForPath(g, buttonPath);
		}
		else {
			// Draw button
			g.setColour(Colour::fromRGBA(0x38, 0x37, 0x3D, 0x88));
			g.fillPath(buttonPath);
		}
	}
};
