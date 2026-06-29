/*
  ==============================================================================

    GUIComponents.cpp
    Created: 8 Jun 2026 7:00:43pm
    Author:  Benjamin Ward (Old Computer)

  ==============================================================================
*/

#include "GUIComponents.h"

//======================================//
//=========== Dial Component ===========//
//======================================//

BasicDialComponent::BasicDialComponent()
{
    dial.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    dial.setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);
    
    dial.addListener(this);
    
    nameLabel.setJustificationType(juce::Justification::centred);
    
    valueLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(nameLabel);
    addAndMakeVisible(dial);
    addAndMakeVisible(valueLabel);

}


void BasicDialComponent::paint(juce::Graphics &g)
{
    if (showOutline) {
        g.setColour(accentColour1);
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 4.0f, 1.0f);
    }
}


void BasicDialComponent::resized()
{
    auto bounds = getLocalBounds();
    auto totalHeight = bounds.getHeight();
    auto quarterHeight = totalHeight * 0.25f;
    auto halfHeight = totalHeight * 0.5;
    
    
    
    auto titleBounds = bounds.removeFromTop(quarterHeight);
//    titleBounds.reduce(titleBounds.getWidth()*(1-titleSizeFactor), 0.0f);
    titleBounds.removeFromTop(titleBounds.getHeight()*(1-titleSizeFactor));
    
    
    auto dialBounds = bounds.removeFromTop(halfHeight);
    int dialBoundsScaleX = dialBounds.getWidth() - (dialBounds.getWidth() * dialScaleFactor);
    int dialBoundsScaleY = dialBounds.getHeight() - dialBounds.getHeight() * dialScaleFactor;
    dialBounds.reduce(dialBoundsScaleX, dialBoundsScaleY);
    
    nameLabel.setBounds(titleBounds);
    dial.setBounds(dialBounds.toNearestInt());
    valueLabel.setBounds(bounds);
}

void BasicDialComponent::setText(juce::String newText)
{
    nameLabel.setText(newText, juce::dontSendNotification);
}


void BasicDialComponent::sliderValueChanged(juce::Slider* slider)
{
    valueLabel.setText(juce::String::formatted("%.2f", slider->getValue()), juce::dontSendNotification);
}


void BasicDialComponent::attach(juce::AudioProcessorValueTreeState &apvts, const juce::String& paramID)
{
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, paramID, dial
    );
    
    valueLabel.setText(juce::String(dial.getValue(), 2), juce::dontSendNotification);
}

void BasicDialComponent::setDialSize(float newSize)
{
    dialScaleFactor = newSize;
}

void BasicDialComponent::setTitleSize(float newSize)
{
    titleSizeFactor = newSize;
}




//======================================//
//=========== Toggle Component =========//
//======================================//
BasicToggleComponent::BasicToggleComponent()
{
    button.setToggleable(true);
    button.setClickingTogglesState(true);
    textLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(textLabel);
    addAndMakeVisible(button);
    
};

void BasicToggleComponent::resized()
{
    auto bounds = getLocalBounds();
    auto textBounds = bounds.removeFromTop(getHeight() * 0.25);
    textBounds.removeFromTop(textBounds.getHeight()*(1.0f-titleSizeFactor));
    
    
    int buttonPadTop = 10;
    
    bounds.removeFromTop(buttonPadTop);

    
    bounds.reduce(bounds.getWidth()/2.0f * (1.0f-buttonSize), 0);
    
    int minDim = juce::jmin(bounds.getWidth(), bounds.getHeight());
    if(isSquare)
        bounds.removeFromBottom(bounds.getHeight() - minDim);
    
    textLabel.setBounds(textBounds);
    button.setBounds(bounds);
    
}


void BasicToggleComponent::setText(juce::String newButtonText)
{
    textLabel.setText(newButtonText, juce::dontSendNotification);
}

void BasicToggleComponent::attach(juce::AudioProcessorValueTreeState &apvts, const juce::String &paramID)
{
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, paramID, button
    );
}

void BasicToggleComponent::setButtonSizeAsFloat(float fractionOfAvailableSpace)
{
    buttonSize = fractionOfAvailableSpace;
}

void BasicToggleComponent::setTitleSize(float newSize)
{
    titleSizeFactor = newSize;
}




//======================================//
//=========== Title Component =========//
//======================================//
TitleWithUnderline::TitleWithUnderline()
{
    titleLabel.setJustificationType(juce::Justification::left);
    titleLabel.setBorderSize(juce::BorderSize<int>(0));
    addAndMakeVisible(titleLabel);
}

void TitleWithUnderline::setText(juce::String newTitle)
{
    titleLabel.setText(newTitle, juce::dontSendNotification);
}

void TitleWithUnderline::resized()
{
    auto titleBounds = getLocalBounds().reduced(padding_sides, 0).removeFromTop(getHeight() - padding_bottom);;
    
    titleBounds.removeFromRight(static_cast<int>(titleBounds.getWidth() * (1.0 - fontProportion)));
    titleBounds.removeFromBottom(static_cast<int>(titleBounds.getHeight() * (1.0 - fontProportion)));
    
    m_titleBounds = titleBounds;
    
    titleLabel.setBounds(titleBounds);
}

void TitleWithUnderline::setUnderlineHeightDelta(int newValue)
{
    lineDelta = newValue;
}

void TitleWithUnderline::paint(juce::Graphics& g)
{
    if(showUnderline)
    {
        auto bounds = getLocalBounds().toFloat();
        
        bounds.removeFromBottom(padding_bottom);
        
        auto& laf = getLookAndFeel();
        auto textColour = laf.findColour(juce::Label::textColourId);
        
        g.setColour(textColour);
        
        
        g.drawLine(m_titleBounds.getX(), m_titleBounds.getBottom() + lineDelta, getLocalBounds().getRight(), m_titleBounds.getBottom() + lineDelta, 1.0f);
    }
}

void TitleWithUnderline::toggleUnderline(bool newDecision)
{
    showUnderline = newDecision;
}

void TitleWithUnderline::setFontSizeAsProportionOfSpace(float newValue)
{
    fontProportion = newValue;
}



//======================================//
//=========== GUI Collection ===========//
//======================================//
GUICollection::GUICollection()
{
    addAndMakeVisible(title);
}

void GUICollection::setCollectionTitle(juce::String newTitle)
{
    title.setText(newTitle);
}

void GUICollection::addDial(BasicDialComponent &dial)
{
    DBG("Added Dial");
//    dials.push_back(&dial);
    controls.push_back(&dial);
    addAndMakeVisible(dial);
}

void GUICollection::addToggle(BasicToggleComponent &toggle)
{
    DBG("Added toggle");
//    toggles.push_back(&toggle);
    controls.push_back(&toggle);
    addAndMakeVisible(toggle);
}

void GUICollection::addComponent(juce::Component &component)
{
    controls.push_back(&component);
    addAndMakeVisible(component);
}


void GUICollection::resized()
{
    
    
    int headerPadH = 0;
    int headerPadV = 0;
    
    auto bounds = getLocalBounds().reduced(headerPadH, headerPadV);
    
    auto header = bounds.removeFromTop(getHeight()/4.0f);
    
    auto titleBounds = header;
    
    title.setBounds(titleBounds);
    
    
    

    //Flex box for componenets
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;
    fb.justifyContent = juce::FlexBox::JustifyContent::center;
    fb.alignItems = juce::FlexBox::AlignItems::center;

    for (auto* control : controls)
    {
        fb.items.add(juce::FlexItem(*control)
                        .withFlex(1.0f)
                        .withHeight(bounds.getHeight())
                        .withMargin(0.0f));
    }

    fb.performLayout(bounds);
    
}

void GUICollection::paint(juce::Graphics& g)
{
    if (showOutline) {
        auto& laf = getLookAndFeel();
        
        g.setColour(laf.findColour(juce::Label::textColourId));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 0.0f, 0.5f);

    }
}
