/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class TableTennisAudioProcessor  : public juce::AudioProcessor, 
                                   public juce::AudioProcessorValueTreeState::Listener 
{
public:
    //==============================================================================
    TableTennisAudioProcessor();
    ~TableTennisAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:

    juce::AudioProcessorValueTreeState treeState;

    /*
    The code below does not accomodate a changing of sample rate

        It currently supports 3 seconds of audio at 192kHz, but does not support
     variable audio just yet. This will be implemented further down the line.

    */

    juce::dsp::DelayLine<float> mDelayLine{ 576000 };

    /*
    The variables below will hold a copy of values from the controls
    They will be updated any time the controls - from the UI, are adjusted.

    This is broken up into the different storage options.
    */

    float mDelayTime = 1000.0f;
    float mFeedback = 0.3f;
    float mVolume = 0.f;
    float mOffsetL = 0.f;
    float mOffsetR = 0.f;
    float mMix = 0.f;
    
    bool  mBypass;

    int mEffectChoice = 0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableTennisAudioProcessor)
};
