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
    FileHighlight(juce::AudioProcessorValueTreeState& vts);
    ~FileHighlight() override;
    
    void paint(juce::Graphics& g) override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void timerCallback() override;
    
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    std::atomic<float> filePos{0.0};
    std::atomic<float> fileLen{1.0};
};

#endif /* Graphics_hpp */
