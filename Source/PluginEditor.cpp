/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Dynamic_ConvolverAudioProcessorEditor::Dynamic_ConvolverAudioProcessorEditor (Dynamic_ConvolverAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts),
    thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open");
    openButton.onClick = [this] {openButtonClicked();};
    
    filePosAttch.reset(new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "filepos", filePosSlider));
    filePosSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filePosSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    
    fileLenAttch.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "filelength", fileLengthSlider));
    fileLengthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    fileLengthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    
    dryWetAttch.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "drywet", dryWetSlider));
    dryWetSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    
    fPosLabel.setText("File Position", juce::dontSendNotification);
    fPosLabel.setJustificationType(juce::Justification::centred);
    fLengthLabel.setText("File Length", juce::dontSendNotification);
    fLengthLabel.setJustificationType(juce::Justification::centred);
    dwLabel.setText("Dry/Wet", juce::dontSendNotification);
    dwLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(&fLengthLabel);
    addAndMakeVisible(&fPosLabel);
    
    addAndMakeVisible(&filePosSlider);
    addAndMakeVisible(&fileLengthSlider);
    
    addAndMakeVisible(&dryWetSlider);
    addAndMakeVisible(&dwLabel);
    
    formatManager.registerBasicFormats();
    thumbnail.addChangeListener(this);
    
    fileHighlight = std::make_unique<FileHighlight>(valueTreeState);
    addAndMakeVisible(*fileHighlight);
    
    
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
        
    auto numKnobs = 3;
    auto knobWidth = 100;
    auto knobHeight = 100;
    auto labelHeight = 50;
    auto knobY = height - (knobHeight + labelHeight);
    auto labelY = height - labelHeight;
    
    auto knobPadding = 10;
    auto knobsX = width/numKnobs  - knobWidth - knobPadding/numKnobs;
    
    juce::Rectangle<int> thumbnailBounds (20, 20, width-40, height-230);
    
    fileHighlight->setBounds(thumbnailBounds);
    
    openButton.setBounds(20, getHeight()-190, getWidth()-40, 20);
    
    
    filePosSlider.setBounds(knobsX, knobY, knobWidth, knobHeight);
    fPosLabel.setBounds(knobsX, labelY, knobWidth, labelHeight);
    
    fileLengthSlider.setBounds(knobsX + knobPadding + knobWidth, knobY, knobWidth, knobHeight);
    fLengthLabel.setBounds(knobsX + knobPadding + knobWidth, labelY, knobWidth, labelHeight);
    
    dryWetSlider.setBounds(knobsX + 2*knobPadding + 2*knobWidth, knobY, knobWidth, knobHeight);
    dwLabel.setBounds(knobsX + 2*knobPadding + 2*knobWidth,labelY, knobWidth, labelHeight);
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
            audioProcessor.d_conv->loadNewIR(file);
        }
    }
    );
}
