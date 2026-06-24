/*
  ==============================================================================

    DynamicConvolutionEffect.h
    Created: 20 Jun 2026 7:19:56pm
    Author:  Benjamin Ward (Old Computer)

  ==============================================================================
*/

#pragma once

#include "DynamicConvolver.h"


//Wrapper Class for Dynamic Convolvutoin class
// Handles JUCE terminology, and IR stereo handling

class DynamicConvolutionEffect
{
public:
    
    DynamicConvolutionEffect(juce::AudioProcessorValueTreeState& vts);
    
    void prepare(int buffsize);
    void loadFileAsIR(juce::File newFile);
    void processBlock(juce::AudioBuffer<float> buffer);
    
    
    
private:
    void normalizeFile();
    
    juce::AudioFormatManager formatManager;
    
    std::vector<float> IRdata;
    juce::AudioBuffer<float>IRbuffer;
    
    std::unique_ptr<DynamicConvolverV2> convEngine;
    std::unique_ptr<DynamicConvolverV2> convEngineR;
    
    bool isIrStereo = false;
};
