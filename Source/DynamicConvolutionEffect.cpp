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
    convEngineR = std::make_unique<DynamicConvolverV2>(vts);
}

void DynamicConvolutionEffect::prepare(int buffsize)
{
    convEngine->prepare(buffsize);
    convEngineR->prepare(buffsize);
}

void DynamicConvolutionEffect::loadFileAsIR(juce::File newFile)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(newFile));
    
    if(reader == nullptr)
    {
        juce::Logger::writeToLog("Error: IR failed to load...\n");
        return;
    }
    
    auto totalIrLength = reader->lengthInSamples;
    
    int numChannels = reader->numChannels;
    
    
    isIrStereo = numChannels == 2 ? true : false;
    
    IRdata.resize(static_cast<size_t>(totalIrLength));
    
    IRbuffer.setSize(numChannels, (int)reader->lengthInSamples);
    
    reader->read(&IRbuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
    
    normalizeFile();

    auto span = std::span<const float>(IRbuffer.getReadPointer(0), IRbuffer.getNumSamples());
    
    convEngine->loadNewIR(span);
    
    //IF stereo IR file, load second channel into second convolution engine
    if (isIrStereo) {
        convEngineR->loadNewIR(std::span<const float>(IRbuffer.getReadPointer(1), IRbuffer.getNumSamples()));
    }

}


void DynamicConvolutionEffect::processBlock(juce::AudioBuffer<float> buffer)
{
    //Copy input in channel 1 to channel 2 to avoid stereo processing issues
    juce::FloatVectorOperations::copy(buffer.getWritePointer(1), buffer.getReadPointer(0), buffer.getNumSamples());
    
    auto span = std::span<float>(buffer.getWritePointer(0), buffer.getNumSamples());
    convEngine->process(span);
    
    if (buffer.getNumChannels() > 1) {
        if(isIrStereo)
            convEngineR->process(std::span<float>(buffer.getWritePointer(1), buffer.getNumSamples()));
        else
            juce::FloatVectorOperations::copy(buffer.getWritePointer(1), buffer.getReadPointer(0), buffer.getNumSamples());
    }

    
}

void DynamicConvolutionEffect::normalizeFile()
{
    auto numSamps = IRbuffer.getNumSamples();
    auto mag = IRbuffer.getMagnitude(0, numSamps);
    IRbuffer.applyGain(0, numSamps, 1/mag);
}
