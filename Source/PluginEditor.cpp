#include "PluginEditor.h"

MySynthPluginAudioProcessorEditor::MySynthPluginAudioProcessorEditor (
    MySynthPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Gain knob
    gainKnob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    gainKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (gainKnob);

    gainLabel.setText ("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (gainLabel);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.apvts, "gain", gainKnob);

    setSize (300, 200);
}

MySynthPluginAudioProcessorEditor::~MySynthPluginAudioProcessorEditor() {}

void MySynthPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e1e2e));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawFittedText ("My Synth Plugin", getLocalBounds().removeFromTop (40),
                      juce::Justification::centred, 1);
}

void MySynthPluginAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (40);            // title space

    gainLabel.setBounds (area.removeFromTop (20));
    gainKnob.setBounds  (area.withSizeKeepingCentre (100, 100));
}
