/*
  ==============================================================================

    DynamicConvolverV2.h
    Created: 19 Jun 2026 7:28:04pm
    Author:  Benjamin Ward (Old Computer)

  ==============================================================================
*/

#pragma once



#include <stdio.h>
#include <JuceHeader.h>
#include <complex.h>
#include <memory.h>
#include "Convolver.hpp"


class DynamicConvolverV2 : juce::AudioProcessorValueTreeState::Listener
{
public:

    DynamicConvolverV2(juce::AudioProcessorValueTreeState& vts);
    
    void prepare(int blockSize);
    
    void process(std::span<float> buffer);
    
    void setNewIR(std::span<const float> newData);
    
    
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    
    
private:
    
    
    //Parameters
    std::atomic<float> filePosition{0.0};
    std::atomic<float> fileLength{1.0};
    std::atomic<float> dryWet{0.5};
    
    FastConvolveV2 convEngine;
    
    juce::AudioProcessorValueTreeState& valueTreeState;

    
    
};
