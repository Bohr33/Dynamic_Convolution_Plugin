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
    void loadNewIR(std::span<const float> newData);
    void process(std::span<float> buffer);
    
    
    
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    
    
private:
    
    
    //From FastConvV2 ============================
    void clearBuffers();
    void createIRfft();
    void resizeMatrix(std::vector<std::vector<float>>& matrix, size_t outside, size_t inside);
    
    //Convolution Functions -- Called by processBlock
    void addNewInputFFT(std::span<float> newFFT);
    void multiplyFFTs(const std::span<float> input, const std::span<float> irFFT, std::span<float> output);
    
    
    
    int bufferSize;
    int fftSize;
    int fftOrder;
    
    int numPartitions = 0;
    
    bool IRloaded = false;
    
    //FFT Object
    std::unique_ptr<juce::dsp::FFT> fft;
    
    //Basic fftBuffer to hold outputs, especially in createWindowedFFT()
    std::vector<float> fftBuffer;
    
    //output FFT Array
//    std::vector<float> summedFFT;
    std::vector<float> windowedFFT;
    
    //IR Raw Data
    std::vector<float> irData; // Will replace above JUCE BUffer
    
    //IR FFT Results / Convolution Buffer
    std::vector<std::vector<float>> IRffts;
    
    //Input FFT Array
    std::vector<std::vector<float>> inputFFTbuffer;
    int inputFftIndex = 0;
    
    //Overlap Buffer
//    juce::AudioBuffer<float> overlapBuffer;
    std::vector<float> overlapBuffer;
    
    
    //End Fast COnv Import==========================
    void createWindowedFFT(int startIndex, int endIndex);
    
    //Parameters
    std::atomic<float> filePosition{0.0};
    std::atomic<float> fileLength{1.0};
    std::atomic<float> dryWet{0.5};
    
    std::atomic<bool> newParams = false;
    
    

    
    juce::AudioProcessorValueTreeState& valueTreeState;

    
    
};
