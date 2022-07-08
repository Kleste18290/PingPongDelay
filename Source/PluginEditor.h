/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class TableTennisAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TableTennisAudioProcessorEditor (TableTennisAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~TableTennisAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
 
    //This allows for the labling and updating of the interfaces' sliders and dials in Real Time

    TableTennisAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& treeState;

    // Juce Contrl variables
    
    juce::Slider delayTimeSlider;
    juce::Slider feedbackSlider;
    juce::Slider gainSlider;
    juce::Slider mixingSlider;
    juce::Slider offsetLSlider;
    juce::Slider offsetRSlider;
    juce::ToggleButton bypassButton;
    juce::ComboBox effectsCombo;


    // Text String variables
    
    juce::Label delayTimeLabel;
    juce::Label feedbackLabel;
    juce::Label gainLabel;
    juce::Label mixingLabel;
    juce::Label offsetLLabel;
    juce::Label offsetRLabel;
    juce::Label bypassLabel;
    juce::Label titleLabel;


    // Scalar values of the Attachments
    
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> feedbackValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> gainValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> mixingValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> offsetLValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> offsetRValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> bypassValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::ComboBoxAttachment> effectsChoice;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableTennisAudioProcessorEditor)
};
