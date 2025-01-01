/*
  ==============================================================================

    LevelMeter.cpp
    Created: 28 Dec 2024 9:22:54pm
    Author:  vivek

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LevelMeter.h"

//==============================================================================
LevelMeter::LevelMeter()
{
    dryRmsValues.resize(bufferSize, 0.0f);
    std::fill(dryRmsValues.begin(), dryRmsValues.end(), mindB);

    wetRmsValues.resize(bufferSize, 0.0f);
    std::fill(wetRmsValues.begin(), wetRmsValues.end(), mindB);
}

LevelMeter::~LevelMeter()
{
}

void LevelMeter::paint (juce::Graphics& g)
{
    // Fill the background
    g.setColour(juce::Colour::fromRGB(0x16, 0x1E, 0x29));
    g.fillRoundedRectangle(backgroundRect, cornerSize);

    // Draw grid lines for dB
    for (int i = mindB + 12; i < maxdB; i += 12) {
        float y = juce::jmap<float>(i, mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());

        if (i == 0) {
            g.setColour(juce::Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x40));
        }
        else {
            g.setColour(juce::Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x10));
        }
        g.drawLine(backgroundRect.getX(), y,backgroundRect.getRight(), y, 0.5 * strokeThickness);
    }

    // Clip the component
    juce::Path clipRegion;
    clipRegion.addRoundedRectangle(backgroundRect, cornerSize);
    g.saveState();
    g.reduceClipRegion(clipRegion);

    // Get dry rms path
    juce::Path dryRmsPath;

    for (int i = 0; i < dryRmsValues.size() - 1; i++) {
        float x = juce::jmap<float>(i, 0, dryRmsValues.size() - 1, backgroundRect.getX(), backgroundRect.getRight());
        float y = juce::jmap<float>(dryRmsValues[i], mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());

        if (i == 0) {
            dryRmsPath.startNewSubPath(x, y);
        }
        else {
            dryRmsPath.lineTo(x, y);
        }
    }

    // Draw dry rms outline
    g.setColour(juce::Colour::fromRGB(0xFF, 0xFF, 0xFF));
    g.strokePath(dryRmsPath, juce::PathStrokeType(strokeThickness / 2.0f));

    // Close dry rms path
    dryRmsPath.lineTo(backgroundRect.getRight(), backgroundRect.getBottom());
    dryRmsPath.lineTo(backgroundRect.getX(), backgroundRect.getBottom());
    dryRmsPath.closeSubPath();

    // Fill dry rms path with gradient
    g.setColour(juce::Colour::fromRGBA(0x55, 0x55, 0x55, 0x88));
    g.fillPath(dryRmsPath);

    if (!bypass) {
        // Get wet rms path
        juce::Path wetRmsPath;

        for (int i = 0; i < wetRmsValues.size() - 1; i++) {
            float x = juce::jmap<float>(i, 0, wetRmsValues.size() - 1, backgroundRect.getX(), backgroundRect.getRight());
            float y = juce::jmap<float>(wetRmsValues[i], mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());

            if (i == 0) {
                wetRmsPath.startNewSubPath(x, y);
            }
            else {
                wetRmsPath.lineTo(x, y);
            }
        }

        // Draw wet rms outline
        g.setColour(juce::Colour::fromRGB(0xFF, 0xFF, 0xFF));
        g.strokePath(wetRmsPath, juce::PathStrokeType(strokeThickness / 2.0f));

        // Close wet rms path
        wetRmsPath.lineTo(backgroundRect.getRight(), backgroundRect.getBottom());
        wetRmsPath.lineTo(backgroundRect.getX(), backgroundRect.getBottom());
        wetRmsPath.closeSubPath();

        // Fill wet rms path with gradient
        g.setGradientFill(waveformGradient);
        g.fillPath(wetRmsPath);
    }

    // Draw dB labels
    for (int i = mindB + 12; i < maxdB; i += 12) {
        float y = juce::jmap<float>(i, mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());

        juce::String text = juce::String(i) + " dB";
        g.setColour(juce::Colour::fromRGB(0xFF, 0xFF, 0xFF));
        g.setFont(10.0f);
        g.drawFittedText(text, juce::Rectangle<int>(backgroundRect.getX(), y - 10, 50, 20), juce::Justification::right, 1);
    }

    // Draw shadow and light for frame
    juce::Path frame;
    juce::PathStrokeType strokeType(5.0f * strokeThickness);
    strokeType.createStrokedPath(frame, clipRegion);
    shadow.drawForPath(g, frame);
    light.drawForPath(g, frame);

    // Draw threshold level bar
    float y = juce::jmap<float>(threshold, mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());
    g.setColour(juce::Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x05));
    g.fillRect(backgroundRect.getX(), y, backgroundRect.getWidth(), backgroundRect.getBottom() - y);
    g.setGradientFill(juce::ColourGradient(radialGradient));
    g.drawLine(backgroundRect.getX(), y, backgroundRect.getRight(), y, strokeThickness);

    // Draw frame
    g.setColour(juce::Colour::fromRGB(0x18, 0x20, 0x2A));
    g.fillPath(frame);

    g.restoreState();
}

void LevelMeter::resized()
{
    backgroundRect = getLocalBounds().toFloat();

    waveformGradient = juce::ColourGradient(juce::Colour::fromRGBA(0x0D, 0x92, 0xF4, 0xAA), backgroundRect.getX(), backgroundRect.getBottom(), juce::Colour::fromRGBA(0xC3, 0x0E, 0x59, 0xAA), backgroundRect.getX(), backgroundRect.getY(), false);
    
    shadow = juce::DropShadow(juce::Colour::fromRGBA(0x00, 0x00, 0x00, 0x44), 5, juce::Point<int>(5, 5));
    light = juce::DropShadow(juce::Colour::fromRGBA(0x40, 0x60, 0x80, 0x20), 5, juce::Point<int>(-5, -5));

    radialGradient = juce::ColourGradient(juce::Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x00), backgroundRect.getX(), 0.0f, juce::Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x00), backgroundRect.getRight(), 0.0f, false);
    radialGradient.addColour(0.5f, juce::Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0xFF));
}

void LevelMeter::addRmsValues(float newDryRmsValue, float newWetRmsValue) {
    std::rotate(dryRmsValues.begin(), dryRmsValues.begin() + 1, dryRmsValues.end());
    dryRmsValues.back() = newDryRmsValue;

    std::rotate(wetRmsValues.begin(), wetRmsValues.begin() + 1, wetRmsValues.end());
    wetRmsValues.back() = newWetRmsValue;

    repaint();
}

void LevelMeter::setThreshold(float newThreshold) {
	threshold = newThreshold;
	repaint();
}

void LevelMeter::setBypass(bool newBypass) {
	bypass = newBypass;
}