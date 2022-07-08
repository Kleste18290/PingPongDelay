/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TableTennisAudioProcessorEditor::TableTennisAudioProcessorEditor (TableTennisAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), treeState(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(450, 750); // 500 = the width of the plugin & 450 = the height of the plugin


    // Bypass Toggle

    bypassValue = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(treeState, "audioBypass", bypassButton);
    bypassButton.setToggleable(true);
    addAndMakeVisible(bypassButton);


    // Effects Selection

     //effectsCombo.addItem("Ping Pong Delay", 1); // Look at ladder filter for comparison of Case-sensitive spelling.
     //effectsCombo.addItem("Delay", 2);
     //effectsCombo.addItem("Echo", 3);
     //effectsChoice = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(treeState, "effectsMode", effectsCombo);
     //addAndMakeVisible(&effectsCombo);


    // Delay time

    delayTimeValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "delayTime", delayTimeSlider);
    delayTimeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    delayTimeSlider.setRange(0.0f, 5000.0f, 1.0f);
    delayTimeSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 75, 25);
    addAndMakeVisible(&delayTimeSlider);

    addAndMakeVisible(delayTimeLabel);
    delayTimeLabel.setText("Delay time (samples)", juce::dontSendNotification);
    delayTimeLabel.attachToComponent(&delayTimeSlider, false);


    // Feedback

    feedbackValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "feedback", feedbackSlider);
    feedbackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    feedbackSlider.setRange(0.0f, 0.99f, 0.01f);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 75, 25);
    addAndMakeVisible(&feedbackSlider);

    addAndMakeVisible(feedbackLabel);
    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.attachToComponent(&feedbackSlider, false);


    // Gain 

    gainValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "gain", gainSlider);
    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setRange(0.0f, 0.99f, 0.01f);
    gainSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 75, 25);
    addAndMakeVisible(&gainSlider);

    addAndMakeVisible(gainLabel);
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.attachToComponent(&gainSlider, false);


    // Panning - Wet Dry control

    mixingValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "mix", mixingSlider);
    mixingSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixingSlider.setRange(0.0f, 1.0f, 0.01f);
    mixingSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 75, 25);
    addAndMakeVisible(&mixingSlider);

    addAndMakeVisible(mixingLabel);
    mixingLabel.setText("Dry-Wet Control", juce::dontSendNotification);
    mixingLabel.attachToComponent(&mixingSlider, true);


    // Offset L
    offsetLValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "offsetL", offsetLSlider);
    offsetLSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    offsetLSlider.setRange(0.0f, 1000.0f, 1.0f);
    offsetLSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 75, 25);
    addAndMakeVisible(&offsetLSlider);

    addAndMakeVisible(offsetLLabel);
    offsetLLabel.setText("Offset L (milliseconds)", juce::dontSendNotification);
    offsetLLabel.attachToComponent(&offsetLSlider, false);

    // Offset R
    offsetLValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "offsetR", offsetRSlider);
    offsetRSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    offsetRSlider.setRange(0.0f, 1000.0f, 1.0f);
    offsetRSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 75, 25);
    addAndMakeVisible(&offsetRSlider);

    addAndMakeVisible(offsetRLabel);
    offsetRLabel.setText("Offset R (milliseconds)", juce::dontSendNotification);
    offsetRLabel.attachToComponent(&offsetRSlider, false);
}

TableTennisAudioProcessorEditor::~TableTennisAudioProcessorEditor()
{
}

//==============================================================================
void TableTennisAudioProcessorEditor::paint(juce::Graphics& g)
{

    // Title - due to the image implementation format getting rid of the original one
    
    titleLabel.setText("Table Tennis", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&titleLabel);

    titleLabel.setBounds(35, 10, 210, 50);

    //g.setFont(30);
    //g.drawFittedText("Table Tennis", 35, 20, 210, 50, juce::Justification::centred, 1, 0.0f);

    juce::Image ttImage = juce::ImageCache::getFromMemory(BinaryData::Table_Tennis_1_png, BinaryData::Table_Tennis_1_pngSize);
    if (!ttImage.isNull())
        g.drawImageWithin(ttImage, 0, 0, 450, 750, 0);
    else
        jassert(!ttImage.isNull()); // This checks if the image is loading correctly and if it is in the right place! 

    //g.setColour(juce::Colours::white);
}

void TableTennisAudioProcessorEditor::resized()
{

    /* 
        This is where the bounds are set for the plugin.
        It uses the following format:

        (int1, int2, int3, int4)

        int1 = Is the starting point for height (offset X-axis)
        int2 = Is the end point for width (offset Y-axis)
        int3 = Is the attachments x scale
        int4 = Is the attachments y scale

        Allowing for placement around the plugin's UI
    */
    //effectsCombo.setBounds(40, 50, 150, 30);
    delayTimeSlider.setBounds(50, 110, 320, 50);
    feedbackSlider.setBounds(50, 180, 320, 50);
    gainSlider.setBounds(50, 250, 320, 50);
    offsetLSlider.setBounds(50, 320, 320, 50);
    offsetRSlider.setBounds(50, 390, 320, 50);
    bypassButton.setBounds(400, 50, 50, 25);
    mixingSlider.setBounds(100, 460, 200, 200);
}
