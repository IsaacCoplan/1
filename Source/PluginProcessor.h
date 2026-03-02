#pragma once

#include <JuceHeader.h>

//==============================================================================
// A simple sound that any SineWaveVoice can play
struct SineWaveSound : public juce::SynthesiserSound
{
    SineWaveSound() {}

    bool appliesToNote    (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

//==============================================================================
// A single sine-wave voice for our synthesiser
struct SineWaveVoice : public juce::SynthesiserVoice
{
    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;
        level        = velocity * 0.15;
        tailOff      = 0.0;

        const double cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        const double cyclesPerSample = cyclesPerSecond / getSampleRate();
        angleDelta = cyclesPerSample * juce::MathConstants<double>::twoPi;
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            if (tailOff == 0.0)
                tailOff = 1.0;
        }
        else
        {
            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioSampleBuffer& outputBuffer,
                          int startSample, int numSamples) override
    {
        if (angleDelta == 0.0)
            return;

        if (tailOff > 0.0)
        {
            while (--numSamples >= 0)
            {
                const float sample = (float) (std::sin (currentAngle) * level * tailOff);

                for (int ch = outputBuffer.getNumChannels(); --ch >= 0;)
                    outputBuffer.addSample (ch, startSample, sample);

                currentAngle += angleDelta;
                ++startSample;

                tailOff *= 0.9999;
                if (tailOff <= 0.005)
                {
                    clearCurrentNote();
                    angleDelta = 0.0;
                    break;
                }
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                const float sample = (float) (std::sin (currentAngle) * level);

                for (int ch = outputBuffer.getNumChannels(); --ch >= 0;)
                    outputBuffer.addSample (ch, startSample, sample);

                currentAngle += angleDelta;
                ++startSample;
            }
        }
    }

private:
    double currentAngle = 0.0;
    double angleDelta   = 0.0;
    double level        = 0.0;
    double tailOff      = 0.0;
};

//==============================================================================
class MySynthPluginAudioProcessor : public juce::AudioProcessor
{
public:
    MySynthPluginAudioProcessor();
    ~MySynthPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool   acceptsMidi()               const override { return true;  }
    bool   producesMidi()              const override { return false; }
    bool   isMidiEffect()              const override { return false; }
    double getTailLengthSeconds()      const override { return 0.0;  }

    //==============================================================================
    int  getNumPrograms()                          override { return 1;    }
    int  getCurrentProgram()                       override { return 0;    }
    void setCurrentProgram (int)                   override {}
    const juce::String getProgramName (int)        override { return {};   }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;

private:
    juce::Synthesiser synth;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MySynthPluginAudioProcessor)
};
