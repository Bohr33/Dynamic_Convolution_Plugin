//
//  DynamicConvolver.hpp
//  Dynamic_Convolver
//
//  Created by Benjamin Ward (Old Computer) on 6/9/25.
//

#ifndef DynamicConvolver_hpp
#define DynamicConvolver_hpp

#include <stdio.h>
#include <JuceHeader.h>
#include <complex.h>


class Dynamic_Convolution : juce::AudioProcessorValueTreeState::Listener
{
public:
    Dynamic_Convolution(juce::AudioProcessorValueTreeState& vts);
    virtual ~Dynamic_Convolution();

    virtual void prepare(int buffsize, double sampleRate);
    virtual void loadNewIR(juce::File file);
    void resizeMatrix(std::vector<std::vector<float>>& matrix, size_t outside, size_t inside);
    
    virtual void createIRfft();
    void clearBuffers();
    virtual void process(juce::AudioBuffer<float>& buffer);
    void addNewInputFFT(std::vector<float>& input);
    void multiplyFFTs(const std::vector<float>& input, const std::vector<float>& irFFT, std::vector<float>& result);
    
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void normalizeFile();
    
private:
    int buffersize;
    int fftSize;
    int fftOrder;
    
    int numPartitions = 0;
    int fftIndex = 0;
    
    bool IRloaded = false;
    
    std::atomic<float> filePosition{0.0};
    std::atomic<float> fileLength{1.0};
    std::atomic<float> dryWet{0.5};
    
    juce::AudioFormatManager formatManager;
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    //FFT Object
    std::unique_ptr<juce::dsp::FFT> fft;
    
    //arrays for FFTs and summed Result
    std::vector<float> FFTbuffer;
    std::vector<float> summedFFT;
    
    //IR Audio and overlap Buffer
    juce::AudioBuffer<float> irAudio;
    juce::AudioBuffer<float> overlapBuffer;
    
    //IR Partition Array and Input Buffer
    std::vector<std::vector<float>> IRffts;
    std::vector<std::vector<float>> inputFFTbuffer;
    int inputFftIndex = 0;
    
};

#endif /* DynamicConvolver_hpp */

