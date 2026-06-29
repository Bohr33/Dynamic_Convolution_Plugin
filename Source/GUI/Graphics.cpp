/*
  ==============================================================================

    Graphics.cpp
    Created: 20 Dec 2025 9:38:39pm
    Author:  Benjamin Ward (Old Computer)

  ==============================================================================
*/

#include "Graphics.h"


MyLookAndFeel::MyLookAndFeel()
{
    //SetGlobal Colors
    setColour(juce::Label::textColourId, juce::Colours::black);
    
    
    myFont = juce::Typeface::createSystemTypefaceFor(BinaryData::ZenMaruGothicLight_ttf, BinaryData::ZenMaruGothicLight_ttfSize);
    
    
    setColour(juce::Slider::backgroundColourId, MyColours::accentC);
    
    setColour(juce::ComboBox::backgroundColourId, MyColours::accentA);
    setColour(juce::ComboBox::textColourId, MyColours::textColour);
    setColour(juce::ComboBox::outlineColourId, MyColours::accentB);
    setColour(juce::ComboBox::arrowColourId, MyColours::accentD);
    
    setColour(juce::PopupMenu::backgroundColourId,            MyColours::accentA);
    setColour(juce::PopupMenu::textColourId,                  MyColours::textColour);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, MyColours::accentC);
    setColour(juce::PopupMenu::highlightedTextColourId,       MyColours::backgroundColour);
    
    setColour(juce::TextButton::buttonColourId, MyColours::accentA);
    setColour(juce::TextButton::buttonOnColourId, MyColours::buttonRed);
    
    
}

void MyLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider)
{
    auto size = juce::jmin(width, height);
    float outlineThickness = 1.0f;
    
    auto bounds = juce::Rectangle<float>(x, y, width, height).withSizeKeepingCentre(size, size).reduced(outlineThickness, outlineThickness);
    auto centre = bounds.getCentre();
    auto radius = bounds.getWidth() / 2.0f;
    
    
    auto fillColour = MyColours::accentC;
    auto outlineColour = MyColours::accentD;
    auto indicatorColour = MyColours::accentA;
    
    g.setColour(fillColour);
    g.fillEllipse(bounds);

    // outline
    g.setColour(outlineColour);
    g.drawEllipse(bounds, outlineThickness);

    // line indicator
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto lineLength = radius * 0.5f;

    juce::Point<float> lineStart = centre.getPointOnCircumference(radius - lineLength, angle);
    juce::Point<float> lineEnd   = centre.getPointOnCircumference(radius, angle);

    g.setColour(indicatorColour);;
    g.drawLine({ lineStart, lineEnd },3.0f);
    
}


void MyLookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &button, const juce::Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f); // inset so outline isn't clipped
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    auto circle = bounds.withSizeKeepingCentre(size, size);

    bool isOn = button.getToggleState();
    
    auto buttonStrokeSize = juce::jmap(size, 20.0f, 200.0f, 1.0f, 6.0f);
    
    auto onColour = button.findColour(juce::TextButton::buttonOnColourId).withAlpha(0.2f);

    // body
    g.setColour(isOn ? onColour : backgroundColour);
    g.fillEllipse(circle);
    
    
    // outline
    g.setColour(MyColours::accentD);
    g.drawEllipse(circle, buttonStrokeSize);

}

juce::Font MyLookAndFeel::getLabelFont(juce::Label& label)
{
    jassert(myFont != nullptr);
    
    return juce::Font(juce::FontOptions{}.withName(myFont->getName()).withStyle(myFont->getStyle()).withHeight(label.getHeight()));
}

juce::Font MyLookAndFeel::getComboBoxFont(juce::ComboBox& comboBox)
{
    return juce::Font(juce::FontOptions{}.withName(myFont->getName()).withStyle(myFont->getStyle()).withHeight(comboBox.getHeight()));
}

juce::Font MyLookAndFeel::getPopupMenuFont()
{
    return juce::Font(juce::FontOptions{}.withName(myFont->getName()).withStyle(myFont->getStyle()).withPointHeight(30));;
}


