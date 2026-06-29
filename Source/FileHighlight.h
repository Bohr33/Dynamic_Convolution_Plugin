//
//  FileHighlight.h
//  Dynamic_Convolver - VST3
//
//  Created by Benjamin Ward (Old Computer) on 6/7/25.
//

#pragma once

#include <stdio.h>

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

class FileHighlight : public juce::Component, juce::AudioProcessorValueTreeState::Listener, juce::Timer
{
public:
    FileHighlight(juce::AudioProcessorValueTreeState& vts);
    ~FileHighlight() override;
    
    void paint(juce::Graphics& g) override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void timerCallback() override;
    
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    std::atomic<float> filePos{0.0};
    std::atomic<float> fileLen{1.0};
};


