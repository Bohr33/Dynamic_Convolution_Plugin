/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once


#include "PluginProcessor.h"
#include "Graphics.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_audio_utils/juce_audio_utils.h>

//==============================================================================
/**
*/
class Dynamic_ConvolverAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                               private juce::ChangeListener
{
public:
    Dynamic_ConvolverAudioProcessorEditor (Dynamic_ConvolverAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~Dynamic_ConvolverAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    void thumbnailChanged();

    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int> bounds);
    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int> bounds);
    
    void openButtonClicked();
    void reverseButtonClicked();
    void drawAudioWaveform();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Dynamic_ConvolverAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;
    
    juce::TextButton openButton;
    
    juce::Slider filePosSlider;
    juce::Label  fPosLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filePosAttch;
    
    juce::Slider fileLengthSlider;
    juce::Label  fLengthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fileLenAttch;
    
    juce::Slider dryWetSlider;
    juce::Label dwLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetAttch;
    
    //Custom Graphics Component
    std::unique_ptr<FileHighlight> fileHighlight;
    
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Dynamic_ConvolverAudioProcessorEditor)
};
