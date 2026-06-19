//
//  Convolver.hpp
//  Dynamic_Convolver - VST3
//
//  Created by Benjamin Ward (Old Computer) on 5/29/25.
//

#ifndef Convolver_hpp
#define Convolver_hpp

#include <stdio.h>
#include <JuceHeader.h>
#include <complex.h>
#include <span>



class FastConvolveV2
{
public:

    void prepare(int buffsize);
    void loadNewIR(std::span<const float> newData);
    void processBlock(std::span<float> buffer);
    
private:
    
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
    int fftIndex = 0;
    
    bool IRloaded = false;
    
    juce::AudioFormatManager formatManager;
    
    //FFT Object
    std::unique_ptr<juce::dsp::FFT> fft;
    
    //arrays for FFTs
    std::vector<float> fftBuffer;
    
    //output FFT Array
    std::vector<float> summedFFT;
    
    //IR Raw Data
    juce::AudioBuffer<float> irAudio;
    std::vector<float> irData; // Will replace above JUCE BUffer
    
    //IR FFT Results / Convolution Buffer
    std::vector<std::vector<float>> IRffts;
    
    //Input FFT Array
    std::vector<std::vector<float>> inputFFTbuffer;
    int inputFftIndex = 0;
    
    //Overlap Buffer
//    juce::AudioBuffer<float> overlapBuffer;
    std::vector<float> overlapBuffer;
};


//void timeDomainConvolution(juce::AudioBuffer<float>& buffer)
//{
//    auto* in = buffer.getReadPointer(0);
//    auto* out = earlyOut.getWritePointer(0);
//
//    for(int i = 0; i < buffer.getNumSamples(); ++i)
//    {
//        for(int j = 0; j < earlyIR.size(); ++j)
//        {
//            if(i - j >= 0)
//                out[j] += earlyIR[j] * in[i - j];
//        }
//    }
//}


#endif /* Convolver_hpp */
