/*
  ==============================================================================

    SpectrumAnalyzer.cpp
    Created: 28 Dec 2024 9:29:46pm
    Author:  vivek

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SpectrumAnalyzer.h"

//==============================================================================
SpectrumAnalyzer::SpectrumAnalyzer()
{
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
}

void SpectrumAnalyzer::paint (juce::Graphics& g)
{
    // Fill the background
    g.setColour(juce::Colour::fromRGB(0x18, 0x17, 0x1D));
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
        g.drawLine(backgroundRect.getX(), y, backgroundRect.getRight(), y, 0.5 * strokeThickness);
    }

    // Draw grid lines for Hz
    for (int frequency : frequencies) {
        float ratio = frequency / (sampleRate / 2.0f);
        float skewedProportion = 0.164f * std::log(443.158f * ratio + 1.0f);
        float x = juce::jmap<float>(skewedProportion, backgroundRect.getX(), backgroundRect.getRight());

        g.setColour(juce::Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x10));
        g.drawLine(x, backgroundRect.getY(), x, backgroundRect.getBottom(), 0.5 * strokeThickness);
    }

    // Clip the component
    juce::Path clipRegion;
    clipRegion.addRoundedRectangle(backgroundRect, cornerSize);
    g.saveState();
    g.reduceClipRegion(clipRegion);

    // Get dry spectrum path
    juce::Path drySpectrumPath;

    for (int i = 0; i < SCOPESIZE - 1; i++) {
        float x = juce::jmap<float>(i, 0, SCOPESIZE - 1, backgroundRect.getX(), backgroundRect.getRight());
        float y = juce::jmap<float>(dryScopeData[i], mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());

        if (i == 0) {
            drySpectrumPath.startNewSubPath(x, y);
        }
        else {
            drySpectrumPath.lineTo(x, y);
        }
    }

    // Draw dry spectrum outline
    g.setColour(juce::Colour::fromRGB(0xFF, 0xFF, 0xFF));
    g.strokePath(drySpectrumPath, juce::PathStrokeType(strokeThickness));

    // Close dry spectrum path
    drySpectrumPath.lineTo(backgroundRect.getRight(), backgroundRect.getBottom());
    drySpectrumPath.lineTo(backgroundRect.getX(), backgroundRect.getBottom());
    drySpectrumPath.closeSubPath();

    // Fill dry spectrum path with gradient
    g.setColour(juce::Colour::fromRGBA(0x55, 0x55, 0x55, 0x88));
    g.fillPath(drySpectrumPath);

    if (!bypass) {
        // Get wet spectrum path
        juce::Path wetSpectrumPath;

        for (int i = 0; i < SCOPESIZE - 1; i++) {
            float x = juce::jmap<float>(i, 0, SCOPESIZE - 1, backgroundRect.getX(), backgroundRect.getRight());
            float y = juce::jmap<float>(wetScopeData[i], mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());

            if (i == 0) {
                wetSpectrumPath.startNewSubPath(x, y);
            }
            else {
                wetSpectrumPath.lineTo(x, y);
            }
        }

        // Draw wet spectrum outline
        g.setColour(juce::Colour::fromRGB(0xFF, 0xFF, 0xFF));
        g.strokePath(wetSpectrumPath, juce::PathStrokeType(strokeThickness));

        // Close wet spectrum path
        wetSpectrumPath.lineTo(backgroundRect.getRight(), backgroundRect.getBottom());
        wetSpectrumPath.lineTo(backgroundRect.getX(), backgroundRect.getBottom());
        wetSpectrumPath.closeSubPath();

        // Fill wet spectrum path with gradient
        g.setGradientFill(spectrumGradient);
        g.fillPath(wetSpectrumPath);
    }

    // Draw Hz labels
    for (int frequency: frequencies) {
        float ratio = frequency / (sampleRate / 2.0f);
        float skewedProportion = 0.164f * std::log(443.158f * ratio + 1.0f);
        float x = juce::jmap<float>(skewedProportion, backgroundRect.getX(), backgroundRect.getRight());

        // Draw the frequency labels
        juce::String text = juce::String(frequency) + " Hz";
        g.setColour(juce::Colour::fromRGB(0xFF, 0xFF, 0xFF));
        g.setFont(10.0f);
        g.drawFittedText(text, juce::Rectangle<int>(x - 25, backgroundRect.getBottom() - 20, 50, 20), juce::Justification::centred, 1);
    }

    // Draw shadow and light for frame
    juce::PathStrokeType strokeType(5.0f * strokeThickness);
    juce::Path frame;
    strokeType.createStrokedPath(frame, clipRegion);
    shadow.drawForPath(g, frame);
    light.drawForPath(g, frame);

    // draw highpass cutoff line at y = 0 dB and x = cutoff frequency,  make it curve at cutoff and go down
    float ratio = cutoff / (sampleRate / 2.0f);
    float skewedProportion = 0.164f * std::log(443.158f * ratio + 1.0f);
    float x = juce::jmap<float>(skewedProportion, backgroundRect.getX(), backgroundRect.getRight());
    float y = juce::jmap<float>(0.0f, mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());

    juce::Path cutoffPath;
    cutoffPath.startNewSubPath(x - 20.0f, backgroundRect.getBottom());
    cutoffPath.quadraticTo(x - 20.0f, y + 40.0f, x, y);
    cutoffPath.lineTo(backgroundRect.getRight(), y);

    juce::Path cutoffRegionPath = cutoffPath;
    cutoffRegionPath.lineTo(backgroundRect.getRight(), backgroundRect.getBottom());
    cutoffRegionPath.closeSubPath();
    g.setColour(juce::Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x05));
    g.fillPath(cutoffRegionPath);

    juce::ColourGradient radialGradient(juce::Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x00), x - 20, 0.0f, juce::Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0x00), backgroundRect.getRight(), 0.0f, false);
    float totalDistance = backgroundRect.getRight() - x + 20.0f;
    float proportionAlongGradient = 20.0f / totalDistance;
    radialGradient.addColour(proportionAlongGradient, juce::Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0xFF));
    g.setGradientFill(radialGradient);
    g.strokePath(cutoffPath, juce::PathStrokeType(strokeThickness));

    // Draw frame
    g.setColour(juce::Colour::fromRGB(0x18, 0x17, 0x1D));
    g.fillPath(frame);

    g.restoreState();
}

void SpectrumAnalyzer::resized()
{
    backgroundRect = getLocalBounds().toFloat();

    float y = juce::jmap<float>(0.0f, mindB, maxdB, backgroundRect.getBottom(), backgroundRect.getY());
    spectrumGradient = juce::ColourGradient(juce::Colour::fromRGBA(0xFF, 0xF0, 0x44, 0xAA), backgroundRect.getX(), backgroundRect.getBottom(), juce::Colour::fromRGBA(0xE4, 0x67, 0x2F, 0xAA), backgroundRect.getX(), y, false);
    
    shadow = juce::DropShadow(juce::Colour::fromRGBA(0x00, 0x00, 0x00, 0x44), 5, juce::Point<int>(5, 5));
    light = juce::DropShadow(juce::Colour::fromRGBA(0x48, 0x47, 0x4D, 0x20), 5, juce::Point<int>(-5, -5));
}

void SpectrumAnalyzer::updateSpectra(float* dryFftData, float* wetFftData, float fftSize) {
    for (int i = 0; i < SCOPESIZE; i++) {
        // Skew the spectrum
        float ratio = i / (float)SCOPESIZE;
        float skewedProportion = (std::exp(ratio / 0.164f) - 1.0f) / 443.158f;
        int fftDataIndex = juce::jlimit<int>(0, juce::roundFloatToInt(fftSize / 2.0f - 1.0f), (skewedProportion * fftSize / 2.0f));

        float dryLevel = juce::Decibels::gainToDecibels(dryFftData[fftDataIndex]) - juce::Decibels::gainToDecibels(fftSize) + juce::Decibels::gainToDecibels(512.0f) + i * 0.05f;
        float wetLevel = juce::Decibels::gainToDecibels(wetFftData[fftDataIndex]) - juce::Decibels::gainToDecibels(fftSize) + juce::Decibels::gainToDecibels(512.0f) + i * 0.05f;

        dryScopeData[i] = 0.5f * juce::jlimit(mindB, maxdB, dryLevel) + 0.5f * dryScopeData[i];
        wetScopeData[i] = 0.5f * juce::jlimit(mindB, maxdB, wetLevel) + 0.5f * wetScopeData[i];
    }

    repaint();
}

void SpectrumAnalyzer::setCutoff(float newCutoff) {
	cutoff = newCutoff;

    repaint();
}

void SpectrumAnalyzer::setBypass(bool newBypass) {
	bypass = newBypass;
}