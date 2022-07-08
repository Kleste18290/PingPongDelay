/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TableTennisAudioProcessor::TableTennisAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, juce::Identifier("PARAMETERS"),
                           { std::make_unique<juce::AudioParameterFloat>("delayTime", "Delay (samples)", 10.f, 5000.f, 1.f), // changed to 10 to stop feedback being heard at 0.
                             std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback 0-1", 0.f, 0.99f, 0.3f),
                             std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.f, 1.f, 0.01f),
                             std::make_unique<juce::AudioParameterFloat>("offsetL", "OffsetL", 0.f, 1000.f, 1.f),
                             std::make_unique<juce::AudioParameterFloat>("offsetR", "OffsetR", 0.f, 1000.f, 1.f),
                             std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.f, 1.f, 0.01f), // starts off at my dry signal
                             std::make_unique<juce::AudioParameterBool>("audioBypass", "Audio Bypass", false),
                             //std::make_unique<juce::AudioParameterChoice>("effectsMode", "Effect Mode", juce::StringArray("ping", "delay", "experimental"), 0),
                           })
#endif
{

    const juce::StringArray params = { "delayTime","feedback","gain", "mix", "offsetL", "offsetR", "audioBypass"}; // "effectsMode"
    for (int i = 0; i <= 7; ++i)
    {
        treeState.addParameterListener(params[i], this);
    }

    /*
    treeState.addParameterListener("delayTime", this);
    treeState.addParameterListener("feedback", this);
    treeState.addParameterListener("gain", this);
    treeState.addParameterListener("mix", this);
    treeState.addParameterListener("offsetL", this);
    treeState.addParameterListener("offsetR", this);
    treeState.addParameterListener("audioBypass", this);
    treeState.addParameterListener("effectsMode", this);
    */

   // panPosition = new juce::AudioParameterFloat("panPosition", "Pan Position", -1.0f, 1.0f, 0.0f);
   // addParameter(panPosition);
}

TableTennisAudioProcessor::~TableTennisAudioProcessor()
{
}

//==============================================================================
const juce::String TableTennisAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TableTennisAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TableTennisAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TableTennisAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TableTennisAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TableTennisAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TableTennisAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TableTennisAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TableTennisAudioProcessor::getProgramName (int index)
{
    return {};
}

void TableTennisAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TableTennisAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    {
        /*
        The Following code utilises DSP module classes;

            This code provides detail about the contextualisation in
            which the coding will be calling: Sampling rate and
            Samples per Block

        This is important because they DelayLine class needs to initialise
        with an interal circular buffer with the correct number of channels.
        */

        juce::dsp::ProcessSpec spec;

        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = samplesPerBlock;
        spec.numChannels = getTotalNumOutputChannels();

        mDelayLine.reset();
        mDelayLine.prepare(spec);
    }
}

void TableTennisAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TableTennisAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TableTennisAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Sets individual Writing Pointers into each channel.

    auto* channelDataL = buffer.getWritePointer(0);
    auto* channelDataR = buffer.getWritePointer(1);


    if (mBypass == false)
    {
        //if (mEffectChoice == 1) // Ping-Pong Delay
        //{
            // 1. Loop runs from 0 to get the number of samples in the block

            for (int i = 0; i < buffer.getNumSamples(); i++)

            {
                /*
                    1 The process below allows for the addition for feedback to our sample.

                    This is a basic PingPong Delay

                    1. Retrieve's the input sample value from the input output buffer
                    Then assign a local variable name 'in'.

                    2. 'pop's a sample from the delayLine buffer on the specified channel and assigning
                    to a local variable named 'temp'.

                    The sample that we read from delayLine is determined by 'mDelayTime'
                    which stores the value from the UI controls.

                    Upon calling 'popSample', it updates the read index position
                    in the buffer automatically.

                    3. 'Push' a sample into the delayLine buffer. The value that we
                    write is equal to the input (in) audio sample mixed with the delayed
                    sample that has just popped from the delayLine buffer scaled by the
                    feedback amount.

                    4. Finally, the input/output buffer of the delayed
                    sample accumulated with the input sample. The output is equal to the
                    delayed signal that is mixed with the original input signal. They are
                    summing two signal values that are scaled by the user and can be controlled
                    by the gain parameter. The implementation of a Dry-Wet control through the 
                    use of the dial in the UI. Giving the user control over how much wet signal 
                    they want or how little.  
                */

                float inL = channelDataL[i]; // 1
                float inR = channelDataR[i];

                float temp1 = mDelayLine.popSample(0, mDelayTime + mOffsetL, true); // 2 The addition of 'false' won't implement the channel and give a dry mix option.
                float temp2 = mDelayLine.popSample(1, mDelayTime + mOffsetR, true);

                mDelayLine.pushSample(0, inL + (temp2 * mFeedback)); // 3
                mDelayLine.pushSample(1, inR + (temp1 * mFeedback));

                channelDataL[i] = ((inL * (1 - mMix)) + (temp2 * mMix)) * mVolume; //4 
                channelDataR[i] = ((inR * (1 - mMix)) + (temp1 * mMix)) * mVolume;
            }
        //}

         /*
        else if (mEffectChoice == 2) // - Standard delay module
        {
            for (int i = 0; i < buffer.getNumSamples(); i++)
            {
                float inL = channelDataL[i]; // 1
                float inR = channelDataR[i];

                float temp1 = mDelayLine.popSample(0, mDelayTime, true); // 2 The addition of 'false' won't implement the channel and give a dry mix option.
                float temp2 = mDelayLine.popSample(1, mDelayTime, true);

                mDelayLine.pushSample(0, inL + (temp2 * mFeedback)); // 3
                mDelayLine.pushSample(1, inR + (temp1 * mFeedback));

                //channelDataL[i] = (inL + temp2) * mVolume; // 4
                //channelDataR[i] = (inR + temp1) * mVolume;
            }
        }

       
        else if (mEffectChoice == 3) // - Echo effect (attempt)

        {
            for (int i = 0; i < buffer.getNumSamples(); i++)
            {
                float inL = channelDataL[i];
                float inR = channelDataR[i];

                float temp1 = mDelayLine.popSample(0, mDelayTime, true); // 2 The addition of 'false' won't implement the channel and give a dry mix option.
                float temp2 = mDelayLine.popSample(0, mDelayTime + 250.f, false);
                float temp3 = mDelayLine.popSample(0, mDelayTime + 500.f, false);

                float temp4 = mDelayLine.popSample(1, mDelayTime, true);
                float temp5 = mDelayLine.popSample(1, mDelayTime + 250.f, false);
                float temp6 = mDelayLine.popSample(1, mDelayTime + 250.f, false);

                mDelayLine.pushSample(0, inL + (temp4 * mFeedback) + (temp5 * mFeedback) + (temp6 * mFeedback)); // 3
                mDelayLine.pushSample(1, inR + (temp1 * mFeedback) + (temp2 * mFeedback) + (temp3 * mFeedback));

                //channelDataL[i] = (inL + temp2) * mVolume; // 4
                //channelDataR[i] = (inR + temp1) * mVolume;
            }
        } */  
    
    }
    
    // Bypass Control - does what it says on the tin

    else if (mBypass == true)
    {
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            channelDataL[i] = channelDataL[i] * 0;
            channelDataR[i] = channelDataR[i] * 0;
        }
    }
}

//==============================================================================
bool TableTennisAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TableTennisAudioProcessor::createEditor()
{
    return new TableTennisAudioProcessorEditor (*this, treeState);
}

//==============================================================================
void TableTennisAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);

    // Gets the values from the last session and uses them in the current one
}

void TableTennisAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(treeState.state.getType()))
            treeState.replaceState(juce::ValueTree::fromXml(*xmlState));

    // Saves values from the users input and stores them till the next session
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TableTennisAudioProcessor();
}

// Function called when parameter is changed
void TableTennisAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{

    // Ping Pong delay parameter values

    if (parameterID == "delayTime")
    {
        mDelayTime = newValue;
    }

    else if (parameterID == "feedback")
    {
        mFeedback = newValue;
    }

    else if (parameterID == "gain")
    {
        mVolume = newValue; 
    }

    else if (parameterID == "offsetL")
    {
        mOffsetL = newValue;
    }

    else if (parameterID == "offsetR")
    {
        mOffsetL = newValue;
    }

    else if (parameterID == "mix")
    {
        mMix = newValue;
    }
       
    // Bypass Parameter values 

    else if (parameterID == "audioBypass")
    {
        mBypass = newValue;
    }
    // Has a bug where you have to bypass the system first before being able to use it.
    //      Almost as if it was resetting the system.


    //else if (parameterID == "effectsMode")
    //{
    //    mEffectChoice = newValue;
    // 
    //}
    /*
        Has a bug where it's implementation affects different parts of the delay module.
    Resulting in it being put on a back burner for now till it's either being troubleshot or 
    is taken off.
    */
}
