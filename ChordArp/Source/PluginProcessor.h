#pragma once
#include <JuceHeader.h>
#include "ChordEngine.h"
#include "ArpEngine.h"
#include "ProgressionPresets.h"

// ─────────────────────────────────────────────────────────────────────────────
class ChordArpProcessor : public juce::AudioProcessor
{
public:
    ChordArpProcessor();
    ~ChordArpProcessor() override;

    // ── AudioProcessor overrides ───────────────────────────────────────────
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ChordArp"; }

    bool   acceptsMidi()  const override { return false; }
    bool   producesMidi() const override { return true;  }
    bool   isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    // ── State accessors (called from editor) ──────────────────────────────
    juce::AudioProcessorValueTreeState& getAPVTS() { return m_apvts; }

    // Read-only access for the editor's visualizer
    const std::vector<ResolvedChord>& getCurrentProgression() const { return m_resolvedProg; }
    int  getCurrentChordIndex() const { return m_chordIndex; }
    int  getCurrentStepIndex()  const { return m_arpEngine.getCurrentStepIndex(); }
    std::vector<int> getArpPreviewSequence() const { return m_arpEngine.getPreviewSequence(); }

    // Called by editor when genre/key/mode/progression selection changes
    void loadProgression (int genreIndex, int progressionIndex);
    void setKey  (int root);
    void setMode (ScaleMode mode);

    // Called by editor on Suggest button press
    void suggestProgression (int genreIndex);

    ChordEngine& getChordEngine() { return m_chordEngine; }

private:
    // ── Parameter layout ──────────────────────────────────────────────────
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // ── Update internal state from APVTS parameter values ─────────────────
    void syncArpSettings();
    void syncChordToArp();

    // ── Members ───────────────────────────────────────────────────────────
    juce::AudioProcessorValueTreeState m_apvts;

    ChordEngine m_chordEngine;
    ArpEngine   m_arpEngine;

    // Resolved progression (rebuilt whenever genre/key/mode changes)
    std::vector<ResolvedChord> m_resolvedProg;
    int m_chordIndex    { 0 };
    int m_suggestionIdx { 0 };

    // Cached copy of current progression (Roman numerals)
    Progression m_currentProg;

    double m_sampleRate { 44100.0 };

    // Transport state: detect stop → reset arp
    bool m_wasPlaying { false };

    // Chord advance: beats-per-chord (default: 4 beats = 1 bar)
    double m_beatsPerChord { 4.0 };
    double m_chordBeatAccum { 0.0 };
    double m_lastPpqForChord { -1.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChordArpProcessor)
};
