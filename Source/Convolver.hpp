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


class Convolution
{
public:

    Convolution()
    {
        formatManager.registerBasicFormats();
    }
    
    void loadNewIR(juce::File file)
    {
        auto& conv = processorChain.template get<convIndex>();
        conv.loadImpulseResponse(file, juce::dsp::Convolution::Stereo::yes,
            juce::dsp::Convolution::Trim::no,
            0);
    }
    
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
        auto& conv = processorChain.template get<convIndex>();
        if(conv.getCurrentIRSize() > 0)
            processorChain.process(context);
    }
    
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        processorChain.prepare(spec);
    }
    
    void reset()
    {
        processorChain.reset();
    }
    
    
private:
    enum
    {
        convIndex
    };
    
    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    
    juce::dsp::ProcessorChain<juce::dsp::Convolution> processorChain;
};

class Fast_Convolve
{
public:
    Fast_Convolve();
    virtual ~Fast_Convolve();

    virtual void prepare(int buffsize, double sampleRate);
    virtual void loadNewIR(juce::File file);
    void resizeMatrix(std::vector<std::vector<float>>& matrix, size_t outside, size_t inside);
    
    virtual void createIRfft();
    void clearBuffers();
    virtual void getNextSampleBlock(juce::AudioBuffer<float>& buffer);
    void addNewInputFFT(std::vector<float>& input);
    void multiplyFFTs(const std::vector<float>& input, const std::vector<float>& irFFT, std::vector<float>& result);
    
private:
    int buffersize;
    int fftSize;
    int fftOrder;
    
    int numPartitions = 0;
    int fftIndex = 0;
    
    bool IRloaded = false;
    
    juce::AudioFormatManager formatManager;
    
    //FFT Object
    std::unique_ptr<juce::dsp::FFT> fft;
    
    //arrays for FFTs
    std::vector<float> FFTbuffer;
    
    //output FFT Array
    std::vector<float> summedFFT;
    
    //IR Container
    juce::AudioBuffer<float> irAudio;
    
    //Partition Array
    std::vector<std::vector<float>> IRffts;
    
    //Input FFT Array
    std::vector<std::vector<float>> inputFFTbuffer;
    int inputFftIndex = 0;
    
    //Overlap Buffer
    juce::AudioBuffer<float> overlapBuffer;
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
