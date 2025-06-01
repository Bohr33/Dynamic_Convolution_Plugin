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
//    Convolution()
//    {
//
//        auto dir = juce::File::getCurrentWorkingDirectory();
//
//
//        juce::String string = dir.loadFileAsString();
//        DBG(string);
//
//        int numTries = 0;
//        while (! dir.getChildFile ("Resources").exists() && numTries++ < 15)
//            dir = dir.getParentDirectory();
//
//        DBG(numTries);
//        DBG("Current Working Directory " + juce::File::getCurrentWorkingDirectory().getFullPathName());
//
//        auto& conv = processorChain.template get<convIndex>();
//        conv.loadImpulseResponse(dir.getChildFile("Resources").getChildFile("Fenway_Stairwell_IR.wav"), juce::dsp::Convolution::Stereo::yes,
//            juce::dsp::Convolution::Trim::no,
//            1024);
//
//    }
    Convolution()
    {
        
        formatManager.registerBasicFormats();
        chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
                                                       juce::File{},
                                                       "*.wav");                     // [7]
        auto chooserFlags = juce::FileBrowserComponent::openMode
                          | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)     // [8]
        {
            auto file = fc.getResult();

            if (file != juce::File{})                                                // [9]
            {
                auto* reader = formatManager.createReaderFor (file);                 // [10]

                if (reader != nullptr)
                {
                    auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
                    readerSource.reset (newSource.release());
                    
                    auto& conv = processorChain.template get<convIndex>();
                    conv.loadImpulseResponse(file, juce::dsp::Convolution::Stereo::yes,
                        juce::dsp::Convolution::Trim::no,
                        0);
                }
            }
        });

    }
    
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
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
