//
//  Convolver.cpp
//  Dynamic_Convolver - VST3
//
//  Created by Benjamin Ward (Old Computer) on 5/29/25.
//

#include "Convolver.hpp"

Fast_Convolve::Fast_Convolve()
{
    formatManager.registerBasicFormats();
};

Fast_Convolve::~Fast_Convolve() = default;

void Fast_Convolve::prepare(int buffsize)
{
    //Set Important Info
    this->bufferSize = buffsize;
    fftSize = buffsize * 2;
    auto sizeTimesTwo = fftSize * 2;
    fftOrder = std::log2(fftSize);
    
    fft = std::make_unique<juce::dsp::FFT>(fftOrder);
    
    //Size Buffers and Arrays
    FFTbuffer.resize(sizeTimesTwo);
    summedFFT.resize(sizeTimesTwo);
    overlapBuffer.setSize(1, buffsize);
}

void Fast_Convolve::loadNewIR(juce::File file)
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

void Fast_Convolve::resizeMatrix(std::vector<std::vector<float>>& matrix, size_t outside, size_t inside)
{
    matrix.resize(outside);
    for(auto& index : matrix)
        index.resize(inside, 0.0);
}

void Fast_Convolve::createIRfft()
{
    //get number or partitions needed, and resize our partitions vector
    //Partition size is 1/2 FFTsize,
    int partitionSize = bufferSize;
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

void Fast_Convolve::clearBuffers()
{
    for(auto& inner : inputFFTbuffer)
        juce::FloatVectorOperations::clear(inner.data(), inner.size());
    
    for(auto& inner : IRffts)
        juce::FloatVectorOperations::clear(inner.data(), inner.size());
    
    juce::FloatVectorOperations::clear(FFTbuffer.data(), FFTbuffer.size());
    juce::FloatVectorOperations::clear(summedFFT.data(), summedFFT.size());
}

void Fast_Convolve::getNextSampleBlock(juce::AudioBuffer<float>& buffer)
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
    for(auto i = 0; i < bufferSize; ++i)
    {
        out[i] = summedFFT[i] + overlap[i];
        out2[i] = summedFFT[i] + overlap[i];
    }
    
    //copy second quarter of array
    auto quarterArray = summedFFT.size()/4 ;
    juce::FloatVectorOperations::copy(overlapBuffer.getWritePointer(0), summedFFT.data() + quarterArray, overlapBuffer.getNumSamples());
}

void Fast_Convolve::addNewInputFFT(std::vector<float>& input)
{
    auto size = inputFFTbuffer.size();
    
    if (inputFftIndex >= size)
        inputFftIndex = 0; // Wrap before use
    
    //copy data from input to buffer
    juce::FloatVectorOperations::copy(inputFFTbuffer[inputFftIndex].data(), input.data(), size);

    inputFftIndex += 1;
}

void Fast_Convolve::multiplyFFTs(const std::vector<float>& input, const std::vector<float>& irFFT, std::vector<float>& result)
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



void FastConvolveV2::loadNewIR(std::span<float> newData)
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
    
    juce::Logger::writeToLog("IR FFT Created Successfully!");
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

