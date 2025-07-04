/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Dynamic_ConvolverAudioProcessor::Dynamic_ConvolverAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    filePosParameter = parameters.getRawParameterValue("filelength");
    fileLengthParameter = parameters.getRawParameterValue("filepos");
    d_conv = std::make_unique<Dynamic_Convolution>(parameters);
}

Dynamic_ConvolverAudioProcessor::~Dynamic_ConvolverAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout Dynamic_ConvolverAudioProcessor::createParameterLayout()
{
    int versionHint = 1;
    
    using namespace juce;
    
    return
    {
        std::make_unique<AudioParameterFloat>(ParameterID {"filelength", versionHint}, "File Length", 0.0f, 1.0f, 1.0f),
        std::make_unique<AudioParameterFloat> (ParameterID{"filepos", versionHint},  "File Pos", 0.0f, 1.0f, 0.0f),
        std::make_unique<AudioParameterFloat>(ParameterID {"drywet", versionHint},
                                              "Dry/Wet", 0.0f, 1.0f, 0.5f)
    };
}

const juce::String Dynamic_ConvolverAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Dynamic_ConvolverAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Dynamic_ConvolverAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Dynamic_ConvolverAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Dynamic_ConvolverAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Dynamic_ConvolverAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Dynamic_ConvolverAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Dynamic_ConvolverAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Dynamic_ConvolverAudioProcessor::getProgramName (int index)
{
    return {};
}

void Dynamic_ConvolverAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Dynamic_ConvolverAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
//    juce::dsp::ProcessSpec spec;
//    spec.sampleRate = getSampleRate();
//    spec.numChannels = 2;
//    spec.maximumBlockSize = getBlockSize();
//    convolver.prepare(spec);
//    convolver.reset();
//    f_conv.prepare(samplesPerBlock, sampleRate);
    d_conv->prepare(samplesPerBlock, sampleRate);
}

void Dynamic_ConvolverAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Dynamic_ConvolverAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Dynamic_ConvolverAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    d_conv->process(buffer);
}

//==============================================================================
bool Dynamic_ConvolverAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Dynamic_ConvolverAudioProcessor::createEditor()
{
    return new Dynamic_ConvolverAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void Dynamic_ConvolverAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Dynamic_ConvolverAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Dynamic_ConvolverAudioProcessor();
}
