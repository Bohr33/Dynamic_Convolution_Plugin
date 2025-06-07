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
    
    Fast_Convolve()
    {
        formatManager.registerBasicFormats();
    };
    
    virtual ~Fast_Convolve() = default;
    
    virtual void prepare(int buffsize, double sampleRate)
    {
        //Set Important Info
        this->buffersize = buffsize;
        fftSize = buffsize * 2;
        auto sizeTimesTwo = fftSize * 2;
        fftOrder = std::log2(fftSize);
        
        fft = std::make_unique<juce::dsp::FFT>(fftOrder);
        
        //Size Buffers and Arrays
        FFTbuffer.resize(sizeTimesTwo);
        summedFFT.resize(sizeTimesTwo);
        overlapBuffer.setSize(1, buffsize);
    }
    
    virtual void loadNewIR(juce::File file)
    {
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        
        if(reader == nullptr)
        {
            juce::Logger::writeToLog("Error: IR failed to load...\n");
            IRloaded = false;
            return;
        }
        
        irAudio.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&irAudio, 0, (int)reader->lengthInSamples, 0, true, true);
        
        juce::Logger::writeToLog("IR loaded successfully!\n");
        createIRfft();
    }
    
    void resizeMatrix(std::vector<std::vector<float>>& matrix, size_t outside, size_t inside)
    {
        matrix.resize(outside);
        for(auto& index : matrix)
            index.resize(inside, 0.0);
    }
    
    virtual void createIRfft()
    {
        //get number or partitions needed, and resize our partitions vector
        //Partition size is 1/2 FFTsize,
        int partitionSize = buffersize;
        int totalSamples = irAudio.getNumSamples();
        
        //Ensure that we get enough partitions to hold any number of samples, i.e. round up numPartitions
        numPartitions = (totalSamples + partitionSize - 1) / partitionSize;
        juce::String pString = juce::String(numPartitions);
        numPartitions = std::min(400, numPartitions);
        
        //Clear all currently loaded Data
        clearBuffers();
        
        //resize matrices
        resizeMatrix(IRffts, (size_t) numPartitions + 1, (size_t) fftSize * 2);
        resizeMatrix(inputFFTbuffer, (size_t) numPartitions + 1, (size_t) fftSize * 2);
        
        auto size = IRffts.size();
        
        juce::String myString = juce::String(size);
        juce::String newString = juce::String(partitionSize);
        juce::String sString = juce::String(totalSamples);
        
        juce::Logger::writeToLog("vector size = " + myString);
        juce::Logger::writeToLog("partitionSize = " + newString);
        juce::Logger::writeToLog("numPartitions = " + pString);
        juce::Logger::writeToLog("num samps = " + sString);
        
        //Copy data from loaded IR and split into partitions
        for(auto i = 0; i < numPartitions; ++i)
        {
            for(auto j = 0; j < partitionSize; ++j)
            {
                auto index = j + i * partitionSize;
                if(index < totalSamples)
                {
                    //only copy first channel for now
                    IRffts[i][j] = irAudio.getSample(0, index);
                }else
                    IRffts[i][j] = 0.0f;
            }
            //perform fft on each partition
            fft->performRealOnlyForwardTransform(IRffts[i].data());
        }
        
        juce::Logger::writeToLog("IR FFT Created Successfully!");
        IRloaded = true;
    }
    
    void clearBuffers()
    {
        for(auto& inner : inputFFTbuffer)
            juce::FloatVectorOperations::clear(inner.data(), inner.size());
        
        for(auto& inner : IRffts)
            juce::FloatVectorOperations::clear(inner.data(), inner.size());
        
        juce::FloatVectorOperations::clear(FFTbuffer.data(), FFTbuffer.size());
        juce::FloatVectorOperations::clear(summedFFT.data(), summedFFT.size());
    }
    
    virtual void getNextSampleBlock(juce::AudioBuffer<float>& buffer)
    {
        jassert(buffer.getNumSamples() == fftSize/2);
        
        if(!IRloaded)
            return;
        
        //zero pad data and copy input
        juce::FloatVectorOperations::clear(FFTbuffer.data(), FFTbuffer.size());
        juce::FloatVectorOperations::copy(FFTbuffer.data(), buffer.getReadPointer(0), buffer.getNumSamples());
        juce::FloatVectorOperations::clear(summedFFT.data(), summedFFT.size());
        
        //perform FFT and add to buffer
        fft->performRealOnlyForwardTransform(FFTbuffer.data());
        addNewInputFFT(FFTbuffer);
        
        //loop through all input FFTS and IR FFTS, offset and multiply
        for(int i = 0;  i < numPartitions; i++)
        {
            auto currentFFTindex = ((inputFftIndex - 1 + numPartitions) - i) % numPartitions;
            multiplyFFTs(inputFFTbuffer[currentFFTindex], IRffts[i], FFTbuffer);
            
            juce::FloatVectorOperations::multiply(FFTbuffer.data(), 1.0/numPartitions, FFTbuffer.size());
            juce::FloatVectorOperations::add(summedFFT.data(), FFTbuffer.data(), FFTbuffer.size());
        }
        
        //perform IFT on sum
        fft->performRealOnlyInverseTransform(summedFFT.data());

        auto* out = buffer.getWritePointer(0);
        auto* out2 = buffer.getWritePointer(1);
        auto* overlap = overlapBuffer.getReadPointer(0);
        
        //Sum result and last overlap
        for(auto i = 0; i < buffersize; ++i)
        {
            out[i] = summedFFT[i] + overlap[i];
            out2[i] = summedFFT[i] + overlap[i];
        }
        
        //copy second quarter of array
        auto quarterArray = summedFFT.size()/4 ;
        juce::FloatVectorOperations::copy(overlapBuffer.getWritePointer(0), summedFFT.data() + quarterArray, overlapBuffer.getNumSamples());
    }
    
    void addNewInputFFT(std::vector<float>& input)
    {
        auto size = inputFFTbuffer.size();
        
        if (inputFftIndex >= size)
            inputFftIndex = 0; // Wrap before use
        
        //copy data from input to buffer
        juce::FloatVectorOperations::copy(inputFFTbuffer[inputFftIndex].data(), input.data(), size);

        inputFftIndex += 1;
    }
    
    void multiplyFFTs(const std::vector<float>& input, const std::vector<float>& irFFT, std::vector<float>& result)
    {
        //First Two data points correspond to DC and Nyquist frequency
        //The rest of the values are interleaved complex values for each bin
        result[0] = input[0] * irFFT[0];
        result[1] = input[1] * irFFT[1];
        
        for(int i = 2; i < input.size(); i += 2)
        {
            auto realA = input[i];
            auto realB = irFFT[i];
            auto imA = input[i+1];
            auto imB = irFFT[i+1];
            
            result[i] = realA * realB - imA * imB;
            result[i+1] = realA * imB + realB * imA;
        }
    }
    
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
