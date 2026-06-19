//
//  Convolver.cpp
//  Dynamic_Convolver - VST3
//
//  Created by Benjamin Ward (Old Computer) on 5/29/25.
//

#include "Convolver.hpp"

//========================================//
//=========== Fast Convolve V2 ===========//
//========================================//
void FastConvolveV2::prepare(int buffsize)
{
    bufferSize = buffsize;
    fftSize = bufferSize * 2;
    
    fftOrder = std::log2(fftSize);
    
    fft = std::make_unique<juce::dsp::FFT>(fftOrder);
    
    //Size Arrays
    fftBuffer.resize(fftSize * 2); //Ensure FFTbuffers are 2x fftSize
    summedFFT.resize(fftSize * 2);
    overlapBuffer.resize(bufferSize);
}



void FastConvolveV2::loadNewIR(std::span<const float> newData)
{
    if (newData.empty())
        return;
    
    irData.assign(newData.begin(), newData.end());
    createIRfft();
}

void FastConvolveV2::createIRfft()
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
    //Not sure why numPartitions + 1
    resizeMatrix(IRffts, (size_t) numPartitions + 1, (size_t) fftSize * 2);
    resizeMatrix(inputFFTbuffer, (size_t) numPartitions + 1, (size_t) fftSize * 2);
    
    size_t numPartitionsPlusOne = IRffts.size();
    
    juce::String pString = juce::String(numPartitions);
    juce::String myString = juce::String(numPartitionsPlusOne);
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
                IRffts[i][j] = irData[index];
            }else
                IRffts[i][j] = 0.0f;
        }
        //perform fft on each partition
        fft->performRealOnlyForwardTransform(IRffts[i].data());
    }
    
    juce::Logger::writeToLog("IR FFT Created Successfully in FastConvolveV2!");
    IRloaded = true;
}


void FastConvolveV2::clearBuffers()
{
    juce::FloatVectorOperations::clear(fftBuffer.data(), fftBuffer.size());
    juce::FloatVectorOperations::clear(summedFFT.data(), summedFFT.size());
    
    for(auto& inner : inputFFTbuffer)
        juce::FloatVectorOperations::clear(inner.data(), inner.size());
    
    for(auto& inner : IRffts)
        juce::FloatVectorOperations::clear(inner.data(), inner.size());
    
    
    juce::FloatVectorOperations::clear(overlapBuffer.data(), overlapBuffer.size());
}


void FastConvolveV2::processBlock(std::span<float> buffer)
{
    if(!IRloaded)
        return;
    
    //zero pad data and copy input
    juce::FloatVectorOperations::clear(fftBuffer.data(), fftBuffer.size());
    juce::FloatVectorOperations::copy(fftBuffer.data(), buffer.data(), buffer.size());
    
    juce::FloatVectorOperations::clear(summedFFT.data(), summedFFT.size());
    
    //perform FFT and add to buffer
    fft->performRealOnlyForwardTransform(fftBuffer.data());
    addNewInputFFT(fftBuffer);
    
    //loop through all input FFTS and IR FFTS, offset and multiply
    for(int i = 0;  i < numPartitions; i++)
    {
        int currentFFTindex = ((inputFftIndex - 1 + numPartitions) - i) % numPartitions;
        multiplyFFTs(inputFFTbuffer[currentFFTindex], IRffts[i], fftBuffer);
        
        juce::FloatVectorOperations::multiply(fftBuffer.data(), 1.0/numPartitions, fftBuffer.size());
        juce::FloatVectorOperations::add(summedFFT.data(), fftBuffer.data(), fftBuffer.size());
    }
    
    //perform IFT on sum
    fft->performRealOnlyInverseTransform(summedFFT.data());

    auto* overlap = overlapBuffer.data();
    
    //Sum result and last overlap to output
    for(auto i = 0; i < bufferSize; ++i)
    {
        buffer[i] = summedFFT[i] + overlap[i];
    }
    
    //copy second quarter of array
    size_t quarterArray = summedFFT.size()/4 ;
    juce::FloatVectorOperations::copy(overlapBuffer.data(), summedFFT.data() + quarterArray, overlapBuffer.size());
}

void FastConvolveV2::addNewInputFFT(std::span<float> newFFT)
{
    
    if (inputFftIndex >= inputFFTbuffer.size())
        inputFftIndex = 0;
    
    //copy data from input to buffer
    juce::FloatVectorOperations::copy(inputFFTbuffer[inputFftIndex].data(), newFFT.data(), newFFT.size());

    inputFftIndex += 1;
    
}

void FastConvolveV2::multiplyFFTs(const std::span<float> input, const std::span<float> irFFT, std::span<float> output)
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
        output[i+1] = realA * imB + realB * imA;
    }
}


void FastConvolveV2::resizeMatrix(std::vector<std::vector<float>> &matrix, size_t outside, size_t inside)
{
    matrix.resize(outside);
    for(auto& index : matrix)
        index.resize(inside, 0.0);
}

