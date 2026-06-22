/*
  ==============================================================================

    DynamicConvolverV2.cpp
    Created: 19 Jun 2026 7:28:04pm
    Author:  Benjamin Ward (Old Computer)

  ==============================================================================
*/

#include "DynamicConvolverV2.h"



DynamicConvolverV2::DynamicConvolverV2(juce::AudioProcessorValueTreeState& vts) : valueTreeState(vts)
{
    valueTreeState.addParameterListener("FILE_POS", this);
    valueTreeState.addParameterListener("FILE_LEN", this);
    valueTreeState.addParameterListener("DRY_WET", this);
}

void DynamicConvolverV2::prepare(int blockSize)
{
    bufferSize = blockSize;
    fftSize = bufferSize * 2;
    
    fftOrder = std::log2(fftSize);
    fft = std::make_unique<juce::dsp::FFT>(fftOrder);
    
    //Resize Arrays
    fftBuffer.resize(fftSize * 2); //Ensure FFTbuffers are 2x fftSize
    windowedFFT.resize(fftSize * 2);
    overlapBuffer.resize(bufferSize);
}



void DynamicConvolverV2::loadNewIR(std::span<const float> newData)
{
    if (newData.empty())
        return;
    
    irData.assign(newData.begin(), newData.end());
    createIRfft();
}

void DynamicConvolverV2::createIRfft()
{
    //get number or partitions needed, and resize our partitions vector
    //Partition size is 1/2 FFTsize,
    int partitionSize = bufferSize;
    int totalSamples = static_cast<int>(irData.size());
    
    //Ensure that we get enough partitions to hold any number of samples, i.e. round up numPartitions
    //Not sure why partion Size - 1, might be unnoticed mistake
    numPartitions = (totalSamples + partitionSize - 1) / partitionSize;
    
    //Arbitrary maximum of 400 partions
    numPartitions = std::min(400, numPartitions);
    
    //Clear all currently loaded Data
    clearBuffers();
    
    //resize matrices
    resizeMatrix(IRffts, (size_t) numPartitions, (size_t) fftSize * 2);
    resizeMatrix(inputFFTbuffer, (size_t) numPartitions, (size_t) fftSize * 2);
    
    juce::String vector = juce::String(IRffts.size());
    juce::String pString = juce::String(numPartitions);
    juce::String newString = juce::String(partitionSize);
    juce::String sString = juce::String(totalSamples);
    
    juce::Logger::writeToLog("vector size = " + vector);
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
                IRffts[i][j] = irData[index];
            }else
                IRffts[i][j] = 0.0f;
        }
        //perform fft on each partition
        fft->performRealOnlyForwardTransform(IRffts[i].data());
    }
    
    juce::Logger::writeToLog("IR FFT Created Successfully in DynamicConvolverV2!");
    IRloaded = true;
}

void DynamicConvolverV2::clearBuffers()
{
    juce::FloatVectorOperations::clear(fftBuffer.data(), fftBuffer.size());
    juce::FloatVectorOperations::clear(windowedFFT.data(), windowedFFT.size());
    
    for(auto& inner : inputFFTbuffer)
        juce::FloatVectorOperations::clear(inner.data(), inner.size());
    
    for(auto& inner : IRffts)
        juce::FloatVectorOperations::clear(inner.data(), inner.size());
    
    juce::FloatVectorOperations::clear(overlapBuffer.data(), overlapBuffer.size());
}

void DynamicConvolverV2::process(std::span<float> buffer)
{
    if(!IRloaded)
        return;
    
    //zero pad data and copy input
    juce::FloatVectorOperations::clear(fftBuffer.data(), fftBuffer.size());
    juce::FloatVectorOperations::copy(fftBuffer.data(), buffer.data(), buffer.size());
    
    juce::FloatVectorOperations::clear(windowedFFT.data(), windowedFFT.size());
    
    //perform FFT and add to buffer
    fft->performRealOnlyForwardTransform(fftBuffer.data());
    addNewInputFFT(fftBuffer);
    
    
    float filePos = filePosition.load();
    float fileLen = fileLength.load();
    float mixAmt = dryWet.load();
    
    //Indexes for Moving File
    int startIndx = filePos * numPartitions;
    int endIndx = fileLen * numPartitions + startIndx;
    endIndx = endIndx > numPartitions ? numPartitions : endIndx;
    
    createWindowedFFT(startIndx, endIndx);
    
    //perform IFT on sum
    fft->performRealOnlyInverseTransform(windowedFFT.data());
    
    //Sum result and last overlap to output
    for(auto i = 0; i < bufferSize; ++i)
    {
        buffer[i] = (windowedFFT[i] + overlapBuffer[i]) * mixAmt + (1.0 - mixAmt) * buffer[i];
    }
    
    //copy second quarter of array
    size_t quarterArray = windowedFFT.size()/4 ;
    juce::FloatVectorOperations::copy(overlapBuffer.data(), windowedFFT.data() + quarterArray, overlapBuffer.size());
}

void DynamicConvolverV2::addNewInputFFT(std::span<float> newFFT)
{
    inputFftIndex += 1;
    
    if (inputFftIndex >= inputFFTbuffer.size())
        inputFftIndex = 0;
    
    //copy data from input to buffer
    juce::FloatVectorOperations::copy(inputFFTbuffer[inputFftIndex].data(), newFFT.data(), newFFT.size());
}

void DynamicConvolverV2::multiplyFFTs(const std::span<float> input, const std::span<float> irFFT, std::span<float> output)
{
    //First Two data points correspond to DC and Nyquist frequency
    //The rest of the values are interleaved complex values for each bin
    output[0] = input[0] * irFFT[0];
    output[1] = input[1] * irFFT[1];
    
    for(int i = 2; i < input.size(); i += 2)
    {
        float realA = input[i];
        float realB = irFFT[i];
        float imA = input[i+1];
        float imB = irFFT[i+1];
        
        output[i] = realA * realB - imA * imB;
        
    }
}

void DynamicConvolverV2::resizeMatrix(std::vector<std::vector<float>> &matrix, size_t outside, size_t inside)
{
    matrix.resize(outside);
    for(auto& index : matrix)
        index.resize(inside, 0.0);
}

void DynamicConvolverV2::createWindowedFFT(int startIndex, int endIndex)
{
    for(int i = startIndex;  i < endIndex; i++)
    {
        auto currentFFTindex = ((inputFftIndex + numPartitions) - (i - startIndex)) % numPartitions;
        
        //Multiply Input with IR, Scale down result, add to window buffer
        multiplyFFTs(inputFFTbuffer[currentFFTindex], IRffts[i], fftBuffer);
    
        juce::FloatVectorOperations::multiply(fftBuffer.data(), 1.0/numPartitions, fftBuffer.size());
        juce::FloatVectorOperations::add(windowedFFT.data(), fftBuffer.data(), fftBuffer.size());
    }
}


void DynamicConvolverV2::parameterChanged(const juce::String& parameterID, float newValue)
{
    if(parameterID == "FILE_POS")
        filePosition.store(newValue);
    else if (parameterID == "FILE_LEN")
        fileLength.store(newValue);
    else if(parameterID == "DRY_WET")
        dryWet.store(newValue);
}
