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
    void convolveWithWindow(int startIndex, int endIndex);
    

    //FFT Object
    std::unique_ptr<juce::dsp::FFT> fft;
    
    int bufferSize;
    int fftSize;
    int fftOrder;
    
    int numPartitions = 0;
    
    bool IRloaded = false;
    
    
    std::vector<float> irData; //IR Raw Data
    
    std::vector<std::vector<float>> IRffts;//IR FFT Results / Convolution Buffer
    

    //Basic fftBuffer to hold outputs, especially in createWindowedFFT()
    std::vector<float> fftBuffer;

    std::vector<float> windowedFFT;//stores summed FFT output after convolution
    

    int inputFftIndex = 0;
    std::vector<std::vector<float>> inputFFTbuffer;//Stores FFT result from new block input
    
    std::vector<float> overlapBuffer; //Stores overlaping convolution data
    
    
    //End Fast Conv Import==========================

    
    //Parameters
    std::atomic<float> filePosition{0.0};
    std::atomic<float> fileLength{1.0};
    std::atomic<float> dryWet{0.5};
    
    std::atomic<bool> newParams = false;

    juce::AudioProcessorValueTreeState& valueTreeState;
};
