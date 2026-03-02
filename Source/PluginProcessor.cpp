#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
MySynthPluginAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Example parameter: master gain (0.0 – 1.0)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "gain", 1 },
        "Gain",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f, 1.0f),
        0.8f));

    return layout;
}

//==============================================================================
MySynthPluginAudioProcessor::MySynthPluginAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // Add voices
    const int numVoices = 8;
    for (int i = 0; i < numVoices; ++i)
        synth.addVoice (new SineWaveVoice());

    synth.addSound (new SineWaveSound());
}

MySynthPluginAudioProcessor::~MySynthPluginAudioProcessor() {}

//==============================================================================
void MySynthPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    ignoreUnused (samplesPerBlock);
}

void MySynthPluginAudioProcessor::releaseResources() {}

bool MySynthPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

//==============================================================================
void MySynthPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // Apply gain parameter
    const float gain = *apvts.getRawParameterValue ("gain");
    buffer.applyGain (gain);
}

//==============================================================================
juce::AudioProcessorEditor* MySynthPluginAudioProcessor::createEditor()
{
    return new MySynthPluginAudioProcessorEditor (*this);
}

//==============================================================================
void MySynthPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MySynthPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MySynthPluginAudioProcessor();
}
