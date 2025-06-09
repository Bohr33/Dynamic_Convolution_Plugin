//
//  Graphics.cpp
//  Dynamic_Convolver - VST3
//
//  Created by Benjamin Ward (Old Computer) on 6/7/25.
//

#include "Graphics.hpp"

FileHighlight::FileHighlight(juce::AudioProcessorValueTreeState& vts) : valueTreeState(vts){
    valueTreeState.addParameterListener("filepos", this);
    valueTreeState.addParameterListener("filelength", this);
    startTimerHz(60);
};

FileHighlight::~FileHighlight()
{
    valueTreeState.removeParameterListener("filepos", this);
    valueTreeState.removeParameterListener("filelength", this);
}

void FileHighlight::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::whitesmoke.withAlpha(0.5f));
    
    int newX = getWidth() * filePos.load();
    int newWidth = getWidth() * fileLen.load();
    
    g.fillRect(newX, 0, newWidth, getHeight());
}

void FileHighlight::parameterChanged(const juce::String& parameterID, float newValue)
{
    if(parameterID == "filepos")
        filePos.store(newValue);
    else if (parameterID == "filelength")
        fileLen.store(newValue);
}

void FileHighlight::timerCallback()
{
    repaint();
}
