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
    {};
    
    void prepare(int buffsize, double sampleRate)
    {
        //eventually this will be a function that can be used to resize the buffers
        
        this->buffersize = buffsize;
        
        fftSize = buffsize * 2;
        fftOrder = std::log2(fftSize);
        
        fft = std::make_unique<juce::dsp::FFT>(fftOrder);
        
        FFTdata.resize(fftSize * 2);
        summedFFT.resize(fftSize * 2);
        outputArray.resize(fftSize);
        
        juce::String size = juce::String(FFTdata.size());
        juce::String order = juce::String(fftOrder);
        juce::Logger::writeToLog("FFT size = " + size);
        juce::Logger::writeToLog("FFT order = " + order);
    }
    
    void loadNewIR(juce::File file)
    {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        
        if(reader == nullptr)
        {
            juce::Logger::writeToLog("Error: IR failed to load...\n");
            IRloaded = false;
            return;
        }
        
        juce::Logger::writeToLog("IR loaded successfully!\n");
        
        irAudio.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&irAudio, 0, irAudio.getNumSamples(), 0, true, true);
        createIRfft();
    }
    
    void resizeMatrix(std::vector<std::vector<float>>& matrix, size_t outside, size_t inside)
    {
        matrix.resize(outside);
        for(auto& index : matrix)
            index.resize(inside);
    }
    
    void createIRfft()
    {
        //get number or partitions needed, and resize our partitions vector
        //Partition size is 1/2 FFTsize,
        int partitionSize = buffersize;
        int totalSamples = irAudio.getNumSamples();
        
        //Ensure that we get enough partitions to hold any number of samples, i.e. round up numPartitions
        numPartitions = (totalSamples + partitionSize - 1) / partitionSize;
        
        //resize matrices
        resizeMatrix(IRffts, (size_t) numPartitions + 1, (size_t) fftSize * 2);
        resizeMatrix(inputFFTbuffer, (size_t) numPartitions + 1, (size_t) fftSize * 2);
        
        
        auto size = IRffts.size();
        
        juce::String myString = juce::String(size);
        juce::String newString = juce::String(partitionSize);
        juce::String pString = juce::String(numPartitions);
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
                {
                    IRffts[i][j] = 0.0f;
                }
            }
            //perform fft on each partition
            fft->performRealOnlyForwardTransform(IRffts[i].data());
        }
        
        juce::Logger::writeToLog("IR FFT Created Successfully!");
    }
    
    void getNextSampleBlock(juce::AudioBuffer<float>& buffer)
    {
        jassert(buffer.getNumSamples() == fftSize/2);
        //1. copy input buffer audio into fft object, zero-pad
        //2. perform fft transform on input
        //3. Store input FFT into circular buffer
        //4. multiply input fft with ir fft
        //5. do overlap-add method and add other necessary audio results
            //5a. do FFT multiplication with previous last input audio and next IIR block
            //5b. repeat for all partitions
            //5c. Perform ift on all relevant partitions
            //5d. sum all convolution results.
        //6. return audio
        
        if(!IRloaded)
        {
            return;
        }
        
        //zero pad data
        std::fill(FFTdata.begin(), FFTdata.end(), 0.0f);
        
        for(auto i = 0; i < buffer.getNumSamples(); ++i)
        {
            FFTdata[i] = buffer.getSample(0, i);
        }
        
        fft->performRealOnlyForwardTransform(FFTdata.data());
        
        addNewInputFFT(FFTdata);
        
        
        std::fill(summedFFT.begin(), summedFFT.end(), 0.0f);
        
        //loop through all input FFTS and IR FFTS, offset and multiply
        for(int i = 0;  i < numPartitions; i++)
        {
            auto currentFFTindex = ((inputFftIndex - 1 + numPartitions) - i) % numPartitions;
            
            multiplyFFTs(inputFFTbuffer[currentFFTindex], IRffts[i], outputArray);
            
            for(int j = 0; j < fftSize; ++j)
            {
                summedFFT[j] += outputArray[j];
            }
        }
        
        //perform IFT on sum
        fft->performRealOnlyInverseTransform(summedFFT.data());
        
        auto* out = buffer.getWritePointer(0);
        auto* out2 = buffer.getWritePointer(1);
        auto* overlap = overlapBuffer.getWritePointer(0);
        
        //Sum result and last overlap
        for(auto i = 0; i < fftSize/2; ++i)
        {
            out[i] = summedFFT[i] + overlap[i];
            out2[i] = summedFFT[i] + overlap[i];
            
        }
        
        //copy first half
        auto halfArray = summedFFT.size()/2;
        std::copy(summedFFT.begin() + halfArray, summedFFT.end(), overlapBuffer.getWritePointer(0));
    }
    
    void addNewInputFFT(std::vector<float>& input)
    {
        std::copy(input.begin(), input.end(), inputFFTbuffer[inputFftIndex].end());
        inputFftIndex += 1;
        
        //If index is greater than array, wrap
        if(inputFftIndex >= inputFFTbuffer.size())
            inputFftIndex = 0;
        
    }
    
    void multiplyFFTs(const std::vector<float>& input, const std::vector<float>& irFFT, std::vector<float>& result)
    {
        result[0] = input[0] * irFFT[0];
        result[1] = input[1] * irFFT[1];
        
        for(int i = 2; i < fftSize; i += 2)
        {
            auto realA = input[i];
            auto realB = irFFT[i];
            auto imA = input[i+1];
            auto imB = irFFT[i+1];
            
            result[i] = realA * realB - imA * imB;
            result[i+1] = realA * imA + realB * imB;
        }
        
    }
    
private:
    
    int buffersize;
    int fftSize;
    int fftOrder;
    
    int numPartitions = 0;
    int fftIndex = 0;
    
    bool IRloaded = false;
    
    //FFT Object
    std::unique_ptr<juce::dsp::FFT> fft;
    
    //arrays for FFTs
    std::vector<float> FFTdata;
    
    //output FFT Array
    std::vector<float> summedFFT;
    
    //IR Container
    juce::AudioBuffer<float> irAudio;
    
    //Partition Array
    std::vector<std::vector<float>> IRffts;
    int IRfftsIndex = 0;
    
    //Input FFT Array
    std::vector<std::vector<float>> inputFFTbuffer;
    int inputFftIndex = 0;
    
    //Output Array
    std::vector<float> outputArray;
    
    //Overlapp Buffer
    juce::AudioBuffer<float> overlapBuffer;
};

#endif /* Convolver_hpp */
