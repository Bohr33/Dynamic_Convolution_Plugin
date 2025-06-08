/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Convolver.hpp"

//==============================================================================
/**
*/
class Dynamic_ConvolverAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Dynamic_ConvolverAudioProcessor();
    ~Dynamic_ConvolverAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    //==============================================================================
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    Convolution convolver;
    Fast_Convolve f_conv;
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Dynamic_ConvolverAudioProcessor)
    
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* filePosParameter = nullptr;
    std::atomic<float>* fileLengthParameter  = nullptr;
    
    
//    juce::dsp::ProcessSpec spec;
//    juce::dsp::Convolution convolution;
    
};
