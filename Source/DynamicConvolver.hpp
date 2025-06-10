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
    
    struct ChannelConvolutionState
    {
        int fftIndex = 0;
        
        std::vector<float> FFTbuffer;
        std::vector<float> summedFFT;
        
        std::vector<std::vector<float>> IrPartitions;
        std::vector<std::vector<float>> inputFFTbuffer;
        
        juce::AudioBuffer<float> overlapBuffer;;
        
        void clearAll()
        {
            juce::FloatVectorOperations::clear(FFTbuffer.data(), FFTbuffer.size());
            juce::FloatVectorOperations::clear(summedFFT.data(), summedFFT.size());
            overlapBuffer.clear();
            
            for(auto& inner : inputFFTbuffer)
                juce::FloatVectorOperations::clear(inner.data(), inner.size());
            
            for(auto& inner : IrPartitions)
                juce::FloatVectorOperations::clear(inner.data(), inner.size());
        }
        
        void clearFFTbuffers()
        {
                juce::FloatVectorOperations::clear(FFTbuffer.data(), FFTbuffer.size());
                juce::FloatVectorOperations::clear(summedFFT.data(), summedFFT.size());
        }
    };
    
    
    Dynamic_Convolution(juce::AudioProcessorValueTreeState& vts);
    virtual ~Dynamic_Convolution();

    virtual void prepare(int buffsize, double sampleRate);
    virtual void loadNewIR(juce::File file);
    void resizeMatrix(std::vector<std::vector<float>>& matrix, size_t outside, size_t inside);
    
    virtual void createIRfft();
    void partitionIR(int partitionSize, int totalSamples, int channel, ChannelConvolutionState& state);
    void clearBuffers();
    virtual void processChannel(int channel, juce::AudioBuffer<float>& buffer);
    void addNewInputFFT(ChannelConvolutionState& convolutionState);
    void multiplyFFTs(const std::vector<float>& input, const std::vector<float>& irFFT, std::vector<float>& result);
    
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void normalizeFile();
    
    void createSummedFFT(int startIndex, int endIndex, ChannelConvolutionState& state);
    
    void storeOverlap(ChannelConvolutionState& state);
    
    void process(juce::AudioBuffer<float>& buffer);
    
    
    
private:
    int buffersize;
    int fftSize;
    int fftOrder;
    
    int numPartitions = 0;
    int IRchannels = 0;
    
    bool IRloaded = false;
    
    std::atomic<float> filePosition{0.0};
    std::atomic<float> fileLength{1.0};
    std::atomic<float> dryWet{0.5};
    
    juce::AudioFormatManager formatManager;
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    

    
    //Convolution State
    std::vector<ChannelConvolutionState> convolutionChannelState;
    
    //FFT Object
    std::unique_ptr<juce::dsp::FFT> fft;
    
    //arrays for FFTs and summed Result
    std::vector<float> FFTbufferL, FFTbufferR;
    std::vector<float> summedFFT;
    
    //IR Audio and overlap Buffer
    juce::AudioBuffer<float> irAudio;
    juce::AudioBuffer<float> overlapBuffer;
    
    //IR Partition Array and Input Buffer
    std::vector<std::vector<float>> IrPartitionsL, IrPartitionsR;
    
    std::vector<std::vector<float>> inputFFTbufferL, inputFFTbufferR;
    
    int inputFftIndex = 0;
    
};

#endif /* DynamicConvolver_hpp */

