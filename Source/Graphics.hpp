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

class FileHighlight : public juce::Component
{
public:
    FileHighlight(){};
    
    void setHighlightedBounds(juce::Rectangle<int> bounds)
    {
        HighlightedArea = bounds;
        repaint(HighlightedArea);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::whitesmoke.withAlpha(0.5f));
        g.fillRect(HighlightedArea);
    }
    
    
private:
    juce::Rectangle<int> HighlightedArea;
};

#endif /* Graphics_hpp */
