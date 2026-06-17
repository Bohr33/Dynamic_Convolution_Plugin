/*
  ==============================================================================

    ConvolutionEffects.h
    Created: 16 Jun 2026 10:23:33pm
    Author:  Benjamin Ward (Old Computer)

  ==============================================================================
*/

#pragma once

#include "Convolver.hpp"

class ConvolutionEffect
{
public:
    
    ConvolutionEffect();
    
    void prepare(int buffsize);
    void loadFileAsIR(juce::File newFile);
    void processBlock(juce::AudioBuffer<float> buffer);
    
    
    
private:
    void normalizeFile();
    
    juce::AudioFormatManager formatManager;
    
    std::vector<float> IRdata;
    juce::AudioBuffer<float>IRbuffer;
    
    FastConvolveV2 convEngine;
};
