/*
  ==============================================================================

    DynamicDynamicConvolutionEffect.cpp
    Created: 20 Jun 2026 7:19:56pm
    Author:  Benjamin Ward (Old Computer)

  ==============================================================================
*/

#include "DynamicConvolutionEffect.h"



DynamicConvolutionEffect::DynamicConvolutionEffect(juce::AudioProcessorValueTreeState& vts)
{
    formatManager.registerBasicFormats();
    convEngine = std::make_unique<DynamicConvolverV2>(vts);
}

void DynamicConvolutionEffect::prepare(int buffsize)
{
    convEngine->prepare(buffsize);
}

void DynamicConvolutionEffect::loadFileAsIR(juce::File newFile)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(newFile));
    
    //turn off flag when a new file is being loaded.
//    IRloaded = false;
    
    if(reader == nullptr)
    {
        juce::Logger::writeToLog("Error: IR failed to load...\n");
        return;
    }
    
    auto totalIrLength = reader->lengthInSamples;
    
    IRdata.resize(static_cast<size_t>(totalIrLength));

    IRbuffer.setSize(1, (int)reader->lengthInSamples);
    reader->read(&IRbuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
    
    normalizeFile();
    
    auto span = std::span<const float>(IRbuffer.getReadPointer(0), IRbuffer.getNumSamples());
    
    convEngine->loadNewIR(span);

}


void DynamicConvolutionEffect::processBlock(juce::AudioBuffer<float> buffer)
{
    
    auto span = std::span<float>(buffer.getWritePointer(0), buffer.getNumSamples());
    convEngine->process(span);
    
}

void DynamicConvolutionEffect::normalizeFile()
{
    auto numSamps = IRbuffer.getNumSamples();
    auto mag = IRbuffer.getMagnitude(0, numSamps);
    IRbuffer.applyGain(0, numSamps, 1/mag);
}
