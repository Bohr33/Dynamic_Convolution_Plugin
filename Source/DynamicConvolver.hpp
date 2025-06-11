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
#include <memory.h>


class Dynamic_Convolution : juce::AudioProcessorValueTreeState::Listener
{
public:
    
    struct ChannelConvolutionState
    {
        int fftIndex = 0;
        
        std::vector<float> FFTbuffer;
        std::vector<float> summedFFT;
        
        std::shared_ptr<std::vector<std::vector<float>>> IrPartitions;
        std::vector<std::vector<float>> inputFFTbuffer;
        
        juce::AudioBuffer<float> overlapBuffer;;
        
        void clearAll()
        {
            clearFFTbuffers();
            overlapBuffer.clear();
            
            for(auto& inner : inputFFTbuffer)
                juce::FloatVectorOperations::clear(inner.data(), inner.size());
            
            if(IrPartitions != nullptr)
            {
                for(auto& inner : *IrPartitions)
                    juce::FloatVectorOperations::clear(inner.data(), inner.size());
            }
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
    void partitionIR(int partitionSize, int totalSamples, int channel, std::vector<std::vector<float>>& matrixToFill);
    
    virtual void processChannel(int channel, juce::AudioBuffer<float>& buffer);
    void addNewInputFFT(ChannelConvolutionState& convolutionState);
    void multiplyFFTs(const std::vector<float>& input, const std::vector<float>& irFFT, std::vector<float>& result);
    
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void normalizeFile();
    
    void createSummedFFT(int startIndex, int endIndex, ChannelConvolutionState& state);
    
    void storeOverlap(ChannelConvolutionState& state);
    
    void process(juce::AudioBuffer<float>& buffer);
    
    float* convolve(juce::AudioBuffer<float>& buffer, int channel, ChannelConvolutionState& convolutionState, float filePos, float fileLen);
    
    
    
private:
    int buffersize;
    int fftSize;
    int fftOrder;
    
    int numPartitions = 0;
    int IRchannels = 0;
    
    bool IRloaded = false;
    
    //Parameters
    std::atomic<float> filePosition{0.0};
    std::atomic<float> fileLength{1.0};
    std::atomic<float> dryWet{0.5};
    
    float currentFilePos;
    float currentFileLength;
    
    juce::AudioFormatManager formatManager;
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    //Convolution State
    std::vector<ChannelConvolutionState> convolutionChannelState;
    
    //FFT Object
    std::unique_ptr<juce::dsp::FFT> fft;
    
    //IR Audio
    juce::AudioBuffer<float> irAudio;
    
};

#endif /* DynamicConvolver_hpp */

