#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MySynthPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MySynthPluginAudioProcessorEditor (MySynthPluginAudioProcessor&);
    ~MySynthPluginAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MySynthPluginAudioProcessor& processorRef;

    // Gain knob + label
    juce::Slider gainKnob;
    juce::Label  gainLabel;

    // Attach slider to the APVTS parameter
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MySynthPluginAudioProcessorEditor)
};
