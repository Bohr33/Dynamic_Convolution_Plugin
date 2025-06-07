/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Graphics.hpp"

//==============================================================================
/**
*/
class Dynamic_ConvolverAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                               private juce::ChangeListener
{
public:
    Dynamic_ConvolverAudioProcessorEditor (Dynamic_ConvolverAudioProcessor&);
    ~Dynamic_ConvolverAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    void thumbnailChanged();

    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int> bounds);
    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int> bounds);
    
    void openButtonClicked();
    
    void drawFileHighlight(juce::Graphics& g);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Dynamic_ConvolverAudioProcessor& audioProcessor;
    
    
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;
    
    juce::TextButton openButton;
    
    juce::Slider filePos;
    juce::Label  fPosLabel;
    juce::Slider fileLength;
    juce::Label  fLengthLabel;
    
    FileHighlight fileHighlight;
    
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Dynamic_ConvolverAudioProcessorEditor)
};
