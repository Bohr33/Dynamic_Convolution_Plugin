/*
  ==============================================================================

    DynamicConvolverV2.cpp
    Created: 19 Jun 2026 7:28:04pm
    Author:  Benjamin Ward (Old Computer)

  ==============================================================================
*/

#include "DynamicConvolverV2.h"



DynamicConvolverV2::DynamicConvolverV2(juce::AudioProcessorValueTreeState& vts) : valueTreeState(vts)
{
    vts.addParameterListener("FILE_POS", this);
    vts.addParameterListener("FILE_LENGTH", this);
    vts.addParameterListener("DRY_WET", this);
}

void DynamicConvolverV2::prepare(int blockSize)
{
    convEngine.prepare(blockSize);
}


void DynamicConvolverV2::process(std::span<float> buffer)
{
    
    convEngine.processBlock(buffer);
}



void DynamicConvolverV2::setNewIR(std::span<const float> newData)
{
    convEngine.loadNewIR(newData);
}
