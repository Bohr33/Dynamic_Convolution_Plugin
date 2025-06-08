//
//  Graphics.hpp
//  Dynamic_Convolver - VST3
//
//  Created by Benjamin Ward (Old Computer) on 6/7/25.
//

#ifndef Graphics_hpp
#define Graphics_hpp

#include <JuceHeader.h>
#include <stdio.h>

class FileHighlight : public juce::Component, juce::AudioProcessorValueTreeState::Listener, juce::Timer
{
public:
    FileHighlight(juce::AudioProcessorValueTreeState& vts) : valueTreeState(vts){
        valueTreeState.addParameterListener("filepos", this);
        valueTreeState.addParameterListener("filelength", this);
        startTimerHz(60);
    };
    
    ~FileHighlight() override
    {
        valueTreeState.removeParameterListener("filepos", this);
        valueTreeState.removeParameterListener("filelength", this);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::whitesmoke.withAlpha(0.5f));
        
        int newX = getWidth() * filePos.load();
        int newWidth = getWidth() * fileLen.load();
        
        g.fillRect(newX, 0, newWidth, getHeight());
    }
    
    void parameterChanged(const juce::String& parameterID, float newValue) override
    {
        if(parameterID == "filepos")
            filePos.store(newValue);
        else if (parameterID == "filelength")
            fileLen.store(newValue);
    }
    
    void timerCallback() override
    {
        repaint();
    }
    
    
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    std::atomic<float> filePos{0.0};
    std::atomic<float> fileLen{1.0};
};

#endif /* Graphics_hpp */
