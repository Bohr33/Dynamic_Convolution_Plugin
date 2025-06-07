/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Dynamic_ConvolverAudioProcessorEditor::Dynamic_ConvolverAudioProcessorEditor (Dynamic_ConvolverAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open");
    openButton.onClick = [this] {openButtonClicked();};
    
    filePos.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filePos.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    filePos.setRange(0.0f, 1.0f, 0.01);
    fileLength.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    fileLength.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    filePos.setRange(0.0f, 1.0f, 0.01);
    
    fPosLabel.setText("File Position", juce::dontSendNotification);
    fPosLabel.setJustificationType(juce::Justification::centred);
    fLengthLabel.setText("File Length", juce::dontSendNotification);
    fLengthLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(&fLengthLabel);
    addAndMakeVisible(&fPosLabel);
    addAndMakeVisible(&filePos);
    addAndMakeVisible(&fileLength);
    
    
    formatManager.registerBasicFormats();
    thumbnail.addChangeListener(this);
    
    setSize (400, 470);
}

Dynamic_ConvolverAudioProcessorEditor::~Dynamic_ConvolverAudioProcessorEditor()
{
}

//==============================================================================
void Dynamic_ConvolverAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto height = getHeight();
    auto width = getWidth();
        
    auto knobWidth = 100;
    auto knobHeight = 100;
    
    auto labelHeight = 50;
    
    auto knobPadding = 10;
    auto knobsX = width/2 - knobWidth - knobPadding/2;
    
    openButton.setBounds(20, getHeight()-190, getWidth()-40, 20);
    filePos.setBounds(knobsX, height - (knobHeight + labelHeight), knobWidth, knobHeight);
    fPosLabel.setBounds(knobsX, height - labelHeight, knobWidth, labelHeight);
    
    fileLength.setBounds(knobsX + knobPadding + knobWidth, height - (knobHeight + labelHeight), knobWidth, knobHeight);
    fLengthLabel.setBounds(knobsX + knobPadding + knobWidth, height - labelHeight, knobWidth, labelHeight);
    
    
}

void Dynamic_ConvolverAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    juce::Rectangle<int> thumbnailBounds (20, 20, getWidth()-40, getHeight()-230);
    
    if(thumbnail.getNumChannels() == 0)
    {
        paintIfNoFileLoaded(g, thumbnailBounds);
    }else
    {
        paintIfFileLoaded(g, thumbnailBounds);
    }
    
    fileHighlight.setHighlightedBounds(thumbnailBounds);
    fileHighlight.paint(g);
}

void Dynamic_ConvolverAudioProcessorEditor::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int> bounds)
{
    g.setColour(juce::Colours::darkcyan);
    g.fillRect(bounds);
    g.setColour(juce::Colours::white);
    g.drawFittedText("No File Loaded", bounds, juce::Justification::centred, 1);
}

void Dynamic_ConvolverAudioProcessorEditor::paintIfFileLoaded(juce::Graphics &g, const juce::Rectangle<int> bounds)
{
    g.setColour(juce::Colours::darkcyan);
    g.fillRect(bounds);
    g.setColour(juce::Colours::whitesmoke);
    thumbnail.drawChannels(g, bounds, 0.0f, thumbnail.getTotalLength(), 1.0f);
}

void Dynamic_ConvolverAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if(source == &thumbnail)
    {
        thumbnailChanged();
    }
}


void Dynamic_ConvolverAudioProcessorEditor::thumbnailChanged()
{
        repaint();
}


void Dynamic_ConvolverAudioProcessorEditor::openButtonClicked()
{
    fileChooser = std::make_unique<juce::FileChooser>("Select an Audio File", juce::File{}, "*.wav");
    
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    
    fileChooser->launchAsync(chooserFlags, [this] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        
        if(file != juce::File{})
        {
            auto* reader = formatManager.createReaderFor(file);
            
            if(reader != nullptr)
            {
                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                thumbnail.setSource(new juce::FileInputSource (file));
            }
            //audioProcessor.convolver.loadNewIR(file);
            audioProcessor.f_conv.loadNewIR(file);
        }
    }
    );
}

void Dynamic_ConvolverAudioProcessorEditor::drawFileHighlight(juce::Graphics& g)
{
    
    
}

