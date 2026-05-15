#include <JuceHeader.h>
#include "LevelMeter.h"

LevelMeter::LevelMeter(Processor& p)
    : processorRef(p)
{
    dryRmsValues.resize(bufferSize, 0.0f);
    std::fill(dryRmsValues.begin(), dryRmsValues.end(), mindB);

    wetRmsValues.resize(bufferSize, 0.0f);
    std::fill(wetRmsValues.begin(), wetRmsValues.end(), mindB);
}

LevelMeter::~LevelMeter()
{
}

void LevelMeter::paint (Graphics& g)
{
    // Fill the background
    g.setColour(Colour(0x18, 0x17, 0x1D));
    g.fillRoundedRectangle(backgroundRect, cornerSize);

    // Draw grid lines for dB
    for (int i = mindB + 12; i < maxdB; i += 12) {
        float y = jmap<float>(i, mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());

        if (i == 0) {
            g.setColour(Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x40));
        }
        else {
            g.setColour(Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x10));
        }
        g.drawLine(backgroundRect.getX(), y,backgroundRect.getRight(), y, 0.5 * strokeThickness);
    }

    // Clip the component
    Path clipRegion;
    clipRegion.addRoundedRectangle(backgroundRect, cornerSize);
    g.saveState();
    g.reduceClipRegion(clipRegion);

    // Get dry rms path
    Path dryRmsPath;

    for (int i = 0; i < dryRmsValues.size() - 1; i++) {
        float x = jmap<float>(i, 0, dryRmsValues.size() - 1, backgroundRect.getX(), backgroundRect.getRight());
        float y = jmap<float>(dryRmsValues[i], mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());

        if (i == 0) {
            dryRmsPath.startNewSubPath(x, y);
        }
        else {
            dryRmsPath.lineTo(x, y);
        }
    }

    // Draw dry rms outline
    g.setColour(Colour(0xFF, 0xFF, 0xFF));
    g.strokePath(dryRmsPath, PathStrokeType(strokeThickness / 2.0f));

    // Close dry rms path
    dryRmsPath.lineTo(backgroundRect.getRight(), backgroundRect.getBottom());
    dryRmsPath.lineTo(backgroundRect.getX(), backgroundRect.getBottom());
    dryRmsPath.closeSubPath();

    // Fill dry rms path
    g.setColour(Colour::fromRGBA(0x55, 0x55, 0x55, 0x88));
    g.fillPath(dryRmsPath);

    if (!processorRef.apvts.getRawParameterValue("bypass")->load()) {
        // Get wet rms path
        Path wetRmsPath;

        for (int i = 0; i < wetRmsValues.size() - 1; i++) {
            float x = jmap<float>(i, 0, wetRmsValues.size() - 1, backgroundRect.getX(), backgroundRect.getRight());
            float y = jmap<float>(wetRmsValues[i], mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());

            if (i == 0) {
                wetRmsPath.startNewSubPath(x, y);
            }
            else {
                wetRmsPath.lineTo(x, y);
            }
        }

        // Draw wet rms outline
        g.setColour(Colour(0xFF, 0xFF, 0xFF));
        g.strokePath(wetRmsPath, PathStrokeType(strokeThickness / 2.0f));

        // Close wet rms path
        wetRmsPath.lineTo(backgroundRect.getRight(), backgroundRect.getBottom());
        wetRmsPath.lineTo(backgroundRect.getX(), backgroundRect.getBottom());
        wetRmsPath.closeSubPath();

        // Fill wet rms path with gradient
        g.setGradientFill(levelGradient);
        g.fillPath(wetRmsPath);
    }

    // Draw dB labels
    for (int i = mindB + 12; i < maxdB; i += 12) {
        float y = jmap<float>(i, mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());

        String text = String(i) + " dB";
        g.setColour(Colour(0xFF, 0xFF, 0xFF));
        g.setFont(10.0f);
        g.drawFittedText(text, Rectangle<int>(backgroundRect.getX(), y - 10, 50, 20), Justification::right, 1);
    }

    // Draw shadow and light for frame
    Path frame;
    PathStrokeType strokeType(5.0f * strokeThickness);
    strokeType.createStrokedPath(frame, clipRegion);
    shadow.drawForPath(g, frame);
    light.drawForPath(g, frame);

    // Draw threshold level bar
    float y = jmap<float>(processorRef.apvts.getRawParameterValue("threshold")->load(), mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());
    g.setColour(Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x05));
    g.fillRect(backgroundRect.getX(), y, backgroundRect.getWidth(), backgroundRect.getBottom() - y);
    g.setGradientFill(juce::ColourGradient(radialGradient));
    g.drawLine(backgroundRect.getX(), y, backgroundRect.getRight(), y, strokeThickness);

    // Draw frame
    g.setColour(Colour(0x18, 0x17, 0x1D));
    g.fillPath(frame);

    g.restoreState();
}

void LevelMeter::resized()
{
    backgroundRect = getLocalBounds().toFloat();

    float y = jmap<float>(0.0f, mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());
    levelGradient = ColourGradient(Colour::fromRGBA(0xFF, 0xF0, 0x44, 0xAA), backgroundRect.getX(), backgroundRect.getBottom(), Colour::fromRGBA(0xE4, 0x67, 0x2F, 0xAA), backgroundRect.getX(), y, false);

    shadow = DropShadow(Colour::fromRGBA(0x00, 0x00, 0x00, 0x44), 5, Point<int>(5, 5));
    light = DropShadow(Colour::fromRGBA(0x48, 0x47, 0x4D, 0x20), 5, Point<int>(-5, -5));

    radialGradient = ColourGradient(Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x00), backgroundRect.getX(), 0.0f, Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x00), backgroundRect.getRight(), 0.0f, false);
    radialGradient.addColour(0.5f, Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0xFF));
}

void LevelMeter::fillRmsValues(float newDryRmsValue, float newWetRmsValue) {
    std::rotate(dryRmsValues.begin(), dryRmsValues.begin() + 1, dryRmsValues.end());
    dryRmsValues.back() = newDryRmsValue;

    std::rotate(wetRmsValues.begin(), wetRmsValues.begin() + 1, wetRmsValues.end());
    wetRmsValues.back() = newWetRmsValue;

    repaint();
}
