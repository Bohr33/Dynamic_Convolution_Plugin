//
//  Convolver.hpp
//  Dynamic_Convolver - VST3
//
//  Created by Benjamin Ward (Old Computer) on 5/29/25.
//

#ifndef Convolver_hpp
#define Convolver_hpp

#include <stdio.h>
#include <JuceHeader.h>


class Convolution
{
public:

    Convolution()
    {
        formatManager.registerBasicFormats();
    }
    
    void loadNewIR(juce::File file)
    {
        auto& conv = processorChain.template get<convIndex>();
        conv.loadImpulseResponse(file, juce::dsp::Convolution::Stereo::yes,
            juce::dsp::Convolution::Trim::no,
            0);
    }
    
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
        auto& conv = processorChain.template get<convIndex>();
        if(conv.getCurrentIRSize() > 0)
            processorChain.process(context);
    }
    
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        processorChain.prepare(spec);
    }
    
    void reset()
    {
        processorChain.reset();
    }
    
    
private:
    enum
    {
        convIndex
    };
    
    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    
    juce::dsp::ProcessorChain<juce::dsp::Convolution> processorChain;
};

#endif /* Convolver_hpp */
