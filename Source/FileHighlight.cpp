//
//  FileHighlight.cpp
//  Dynamic_Convolver - VST3
//
//  Created by Benjamin Ward (Old Computer) on 6/7/25.
//

#include "FileHighlight.h"

FileHighlight::FileHighlight(juce::AudioProcessorValueTreeState& vts) : valueTreeState(vts){
    valueTreeState.addParameterListener("FILE_POS", this);
    valueTreeState.addParameterListener("FILE_LEN", this);
    startTimerHz(60);
};

FileHighlight::~FileHighlight()
{
    valueTreeState.removeParameterListener("FILE_POS", this);
    valueTreeState.removeParameterListener("FILE_LEN", this);
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
    if(parameterID == "FILE_POS")
        filePos.store(newValue);
    else if (parameterID == "FILE_LEN")
        fileLen.store(newValue);
}

void FileHighlight::timerCallback()
{
    repaint();
}
