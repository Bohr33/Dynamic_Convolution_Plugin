//
//  DynamicConvolver.cpp
//  Dynamic_Convolver
//
//  Created by Benjamin Ward (Old Computer) on 6/9/25.
//

#include "DynamicConvolver.hpp"

Dynamic_Convolution::Dynamic_Convolution(juce::AudioProcessorValueTreeState& vts) : valueTreeState(vts)
{
    formatManager.registerBasicFormats();
    valueTreeState.addParameterListener("filepos", this);
    valueTreeState.addParameterListener("filelength", this);
    valueTreeState.addParameterListener("drywet", this);
};

Dynamic_Convolution::~Dynamic_Convolution() = default;

void Dynamic_Convolution::process(juce::AudioBuffer<float>& buffer)
{
    for(auto i = 0; i < buffer.getNumChannels(); ++i)
        processChannel(i, buffer);
}

void Dynamic_Convolution::prepare(int buffsize, double sampleRate)
{
    //Set Important Info
    this->buffersize = buffsize;
    fftSize = buffsize * 2;
    auto sizeTimesTwo = fftSize * 2;
    fftOrder = std::log2(fftSize);
    
    fft = std::make_unique<juce::dsp::FFT>(fftOrder);

    
    //Size Buffers and Arrays
    
    //Size for Stereo
    convolutionChannelState.resize(2);
    for(ChannelConvolutionState& channel : convolutionChannelState)
    {
        channel.FFTbuffer.resize(sizeTimesTwo);
        channel.summedFFT.resize(sizeTimesTwo);
        channel.overlapBuffer.setSize(1, buffsize);
    }
        
    
    
//    convolutionChannelState.data()->FFTbuffer.resize(sizeTimesTwo);
//    convolutionChannelState.data()->summedFFT.resize(sizeTimesTwo);
//    convolutionChannelState.data()->overlapBuffer.setSize(1,buffsize);
    
//    FFTbufferL.resize(sizeTimesTwo);
//    summedFFT.resize(sizeTimesTwo);
//    overlapBuffer.setSize(1, buffsize);
}

void Dynamic_Convolution::loadNewIR(juce::File file)
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
    normalizeFile();
    
    juce::Logger::writeToLog("IR loaded successfully!\n");
    createIRfft();
}

void Dynamic_Convolution::normalizeFile()
{
    auto numSamps = irAudio.getNumSamples();
    auto mag = irAudio.getMagnitude(0, numSamps);
    irAudio.applyGain(0, numSamps, 1/mag);
}

void Dynamic_Convolution::resizeMatrix(std::vector<std::vector<float>>& matrix, size_t outside, size_t inside)
{
    matrix.resize(outside);
    for(auto& index : matrix)
        index.resize(inside, 0.0);
}

void Dynamic_Convolution::createIRfft()
{
    //get number or partitions needed, and resize our partitions vector
    //Partition size is 1/2 FFTsize,
    int partitionSize = buffersize;
    int totalSamples = irAudio.getNumSamples();
    IRchannels = irAudio.getNumChannels();
    
    //Ensure that we get enough partitions to hold any number of samples, i.e. round up numPartitions
    numPartitions = (totalSamples + partitionSize - 1) / partitionSize;
    numPartitions = std::min(400, numPartitions);
    
    //Clear all currently loaded Data
    //clearBuffers();
    
    
    
    for(ChannelConvolutionState& channel : convolutionChannelState)
    {
        channel.clearAll();
        resizeMatrix(channel.IrPartitions, (size_t) numPartitions + 1, (size_t) fftSize * 2);
        resizeMatrix(channel.inputFFTbuffer, (size_t) numPartitions + 1, (size_t) fftSize * 2);
    }
    
    //resize matrices
//    resizeMatrix(IrPartitionsL, (size_t) numPartitions + 1, (size_t) fftSize * 2);
//    resizeMatrix(inputFFTbufferL, (size_t) numPartitions + 1, (size_t) fftSize * 2);
    
    juce::String size = juce::String(convolutionChannelState.data()->IrPartitions.size());
    juce::String fftSize = juce::String(convolutionChannelState.data()->FFTbuffer.size());
    
    DBG("IR Partition Size = " + size);
    DBG("FFT Buffer Size = " + fftSize);
    
    
    //Copy data from loaded IR and split into partitions
    for(auto i = 0; i < IRchannels; ++i)
        partitionIR(partitionSize, totalSamples, i, convolutionChannelState[i]);
    
    DBG("Not HERE!");
        
//    partitionIR(partitionSize, totalSamples, 0, IrPartitionsL);
//    if (IRchannels > 1)
//        partitionIR(partitionSize, totalSamples, 1, IrPartitionsR);
    
    juce::Logger::writeToLog("IR FFT Created Successfully!");
    IRloaded = true;
}

void Dynamic_Convolution::partitionIR(int partitionSize, int totalSamples, int channel, ChannelConvolutionState& state)
{
    std::vector<std::vector<float>>& partitionBuffer = state.IrPartitions;
    
    for(auto i = 0; i < numPartitions; ++i)
    {
        for(auto j = 0; j < partitionSize; ++j)
        {
            auto index = j + i * partitionSize;
            if(index < totalSamples)
            {
                //only copy first channel for now
                partitionBuffer[i][j] = irAudio.getSample(channel, index);
            }else
                partitionBuffer[i][j] = 0.0f;
        }
        //perform fft on each partition
        fft->performRealOnlyForwardTransform(partitionBuffer[i].data());
    }
}

void Dynamic_Convolution::clearBuffers()
{
    
    for(auto& inner : inputFFTbufferL)
        juce::FloatVectorOperations::clear(inner.data(), inner.size());
    
    for(auto& inner : IrPartitionsL)
        juce::FloatVectorOperations::clear(inner.data(), inner.size());
    
    for(auto& inner : IrPartitionsR)
        juce::FloatVectorOperations::clear(inner.data(), inner.size());
    
    juce::FloatVectorOperations::clear(FFTbufferL.data(), FFTbufferL.size());
    juce::FloatVectorOperations::clear(summedFFT.data(), summedFFT.size());
}

void Dynamic_Convolution::processChannel(int channel, juce::AudioBuffer<float>& buffer)
{
    jassert(buffer.getNumSamples() == fftSize/2);
    
    if(!IRloaded)
        return;
    
    
    
    float filePos = filePosition.load();
    float fileLen = fileLength.load();
    float dw = dryWet.load();
    
    
    ChannelConvolutionState& state = convolutionChannelState[channel];
    
    std::vector<float>& fftBuffer = state.FFTbuffer;
    
    
    
    //zero pad data and copy input
//    juce::FloatVectorOperations::clear(FFTbufferL.data(), FFTbufferL.size());
//    juce::FloatVectorOperations::clear(summedFFT.data(), summedFFT.size());
    juce::FloatVectorOperations::clear(state.FFTbuffer.data(), state.FFTbuffer.size());
    juce::FloatVectorOperations::clear(state.summedFFT.data(), state.summedFFT.size());
    
    DBG("not here either...");
    
    //state.clearFFTbuffers();
    
    
    
    juce::FloatVectorOperations::copy(fftBuffer.data(), buffer.getReadPointer(channel), buffer.getNumSamples());
    
    
    DBG("nope, not here");
    
    
    //perform FFT and add to buffer
    fft->performRealOnlyForwardTransform(fftBuffer.data());
    addNewInputFFT(state);
    
    
    
    
    
    //Indexes for Moving File
    int startIndx = filePos * numPartitions;
    int endIndx = fileLen * numPartitions + startIndx;
    endIndx = endIndx > numPartitions ? numPartitions : endIndx;
    
    
    createSummedFFT(startIndx, endIndx, state);
    
    
    
//    for(int i = startIndx;  i < endIndx; i++)
//    {
//        auto currentFFTindex = ((state.fftIndex - 1 + numPartitions) - (i - startIndx)) % numPartitions;
//
//        multiplyFFTs(inputFFTbufferL[currentFFTindex], IrPartitionsL[i], FFTbufferL);
//
//        juce::FloatVectorOperations::multiply(FFTbufferL.data(), 1.0/numPartitions, FFTbufferL.size());
//        juce::FloatVectorOperations::add(summedFFT.data(), FFTbufferL.data(), FFTbufferL.size());
//    }
    
    //perform IFT on sum
    fft->performRealOnlyInverseTransform(state.summedFFT.data());

    auto* in = buffer.getWritePointer(channel);
    auto* out = buffer.getWritePointer(channel);
    auto* overlap = state.overlapBuffer.getReadPointer(0);
    
    //Sum result and last overlap
    for(auto i = 0; i < buffersize; ++i)
    {
        out[i] = (state.summedFFT[i] + overlap[i]) * dw + in[i]* (1-dw);
    }
    
    storeOverlap(state);
    
    
    
    
    //copy second quarter of array
//    auto quarterArray = summedFFT.size()/4 ;
//    juce::FloatVectorOperations::copy(overlapBuffer.getWritePointer(0), summedFFT.data() + quarterArray, overlapBuffer.getNumSamples());
}

void Dynamic_Convolution::createSummedFFT(int startIndex, int endIndex, ChannelConvolutionState& state)
{
    for(int i = startIndex;  i < endIndex; i++)
    {
        auto currentFFTindex = ((state.fftIndex - 1 + numPartitions) - (i - startIndex)) % numPartitions;
        
//        multiplyFFTs(inputFFTbufferL[currentFFTindex], IrPartitionsL[i], FFTbufferL);
        
        multiplyFFTs(state.inputFFTbuffer[currentFFTindex], state.IrPartitions[i], state.FFTbuffer);
        
        juce::FloatVectorOperations::multiply(state.FFTbuffer.data(), 1.0/numPartitions, state.FFTbuffer.size());
        juce::FloatVectorOperations::add(state.summedFFT.data(), state.FFTbuffer.data(), state.FFTbuffer.size());
    }
}

void Dynamic_Convolution::storeOverlap(ChannelConvolutionState& state)
{
    auto quarterArray = state.summedFFT.size()/4 ;
    juce::FloatVectorOperations::copy(state.overlapBuffer.getWritePointer(0), state.summedFFT.data() + quarterArray, state.overlapBuffer.getNumSamples());
}

void Dynamic_Convolution::addNewInputFFT(ChannelConvolutionState& state)
{
    auto size = state.inputFFTbuffer.size();
    
    if(state.fftIndex >= size)
        state.fftIndex = 0;
    
    juce::FloatVectorOperations::copy(state.inputFFTbuffer[state.fftIndex].data(), state.FFTbuffer.data(), size);
    state.fftIndex += 1;
//    auto size = inputFFTbufferL.size();
//
//    if (inputFftIndex >= size)
//        inputFftIndex = 0; // Wrap before use
//
//    //copy data from input to buffer
//    juce::FloatVectorOperations::copy(inputFFTbufferL[inputFftIndex].data(), input.data(), size);
//
//    inputFftIndex += 1;
}

void Dynamic_Convolution::multiplyFFTs(const std::vector<float>& input, const std::vector<float>& irFFT, std::vector<float>& result)
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

void Dynamic_Convolution::parameterChanged(const juce::String& parameterID, float newValue)
{
    if(parameterID == "filepos")
        filePosition.store(newValue);
    else if (parameterID == "filelength")
        fileLength.store(newValue);
    else if(parameterID == "drywet")
        dryWet.store(newValue);
}
