/*
  ==============================================================================

    Graphics.h
    Created: 20 Dec 2025 9:38:39pm
    Author:  Benjamin Ward (Old Computer)

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

#include "BinaryData.h"


namespace MyColours
{
const juce::Colour backgroundColour  { 0xFFF0E7D5 };
const juce::Colour textColour   { 0xFF000000 }; //Black

const juce::Colour accentA  { 0xFFCCC5B6 };
const juce::Colour accentB  { 0xFFA39D91 };
const juce::Colour accentC  { 0xFF7A766D };
const juce::Colour accentD  { 0xFF524F49 };

const juce::Colour buttonRed {0xFFF00A00 };
}

class MyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    
    MyLookAndFeel();
    
    void drawRotarySlider(juce::Graphics &g,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider & ) override;
    
    
    void drawButtonBackground    (    juce::Graphics &    ,
                                        juce::Button &    ,
                                  const juce::Colour &    backgroundColour,
                                  bool    shouldDrawButtonAsHighlighted,
                                  bool    shouldDrawButtonAsDown ) override;
    
    juce::Font getLabelFont(juce::Label& label) override;
    juce::Font getComboBoxFont(juce::ComboBox& comboBox) override;

    juce::Font getPopupMenuFont() override;
    
    juce::Font getMyFont();
    

    
    
private:
    juce::Typeface::Ptr myFont;
};



