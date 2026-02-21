#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// ─────────────────────────────────────────────────────────────────────────────
// ChordBlockComponent — one chord label block in the progression visualizer
// ─────────────────────────────────────────────────────────────────────────────
class ChordBlockComponent : public juce::Component
{
public:
    ChordBlockComponent();
    void setLabel    (const juce::String& text);
    void setActive   (bool active);
    void paint (juce::Graphics& g) override;

private:
    juce::String m_label;
    bool         m_active { false };
};

// ─────────────────────────────────────────────────────────────────────────────
// ProgressionVisualizerComponent — horizontal row of ChordBlockComponents
// ─────────────────────────────────────────────────────────────────────────────
class ProgressionVisualizerComponent : public juce::Component
{
public:
    ProgressionVisualizerComponent();
    void setChords      (const std::vector<ResolvedChord>& chords);
    void setActiveChord (int index);
    void resized() override;
    void paint (juce::Graphics& g) override;

private:
    juce::OwnedArray<ChordBlockComponent> m_blocks;
};

// ─────────────────────────────────────────────────────────────────────────────
// PianoRollPreviewComponent — shows the arp note pattern for the active chord
// ─────────────────────────────────────────────────────────────────────────────
class PianoRollPreviewComponent : public juce::Component
{
public:
    PianoRollPreviewComponent();
    void setNotes (const std::vector<int>& midiNotes, int highlightStep);
    void paint (juce::Graphics& g) override;

private:
    std::vector<int> m_notes;
    int              m_highlightStep { -1 };

    static const int k_numOctaves = 2;  // display range
};

// ─────────────────────────────────────────────────────────────────────────────
// LabeledComboBox — a ComboBox with a label above it
// ─────────────────────────────────────────────────────────────────────────────
class LabeledComboBox : public juce::Component
{
public:
    explicit LabeledComboBox (const juce::String& labelText);
    void resized() override;

    juce::ComboBox& getComboBox() { return m_combo; }

private:
    juce::Label    m_label;
    juce::ComboBox m_combo;
};

// ─────────────────────────────────────────────────────────────────────────────
// LabeledSlider — a Slider with a label above it
// ─────────────────────────────────────────────────────────────────────────────
class LabeledSlider : public juce::Component
{
public:
    explicit LabeledSlider (const juce::String& labelText);
    void resized() override;

    juce::Slider& getSlider() { return m_slider; }

private:
    juce::Label  m_label;
    juce::Slider m_slider;
};

// ─────────────────────────────────────────────────────────────────────────────
// ChordArpEditor — main plugin editor window
// ─────────────────────────────────────────────────────────────────────────────
class ChordArpEditor : public juce::AudioProcessorEditor,
                       public juce::Timer
{
public:
    explicit ChordArpEditor (ChordArpProcessor&);
    ~ChordArpEditor() override;

    void paint   (juce::Graphics&) override;
    void resized () override;

    // Timer: refresh visualizer + piano roll in real time
    void timerCallback() override;

private:
    ChordArpProcessor& m_proc;

    // ── Top row: genre, key, mode, suggest ────────────────────────────────
    LabeledComboBox m_genreBox    { "Genre" };
    LabeledComboBox m_progressionBox { "Progression" };
    LabeledComboBox m_keyBox      { "Root Key" };
    LabeledComboBox m_modeBox     { "Mode" };
    juce::TextButton m_suggestBtn { "Suggest" };

    // ── Visualizer + piano roll ────────────────────────────────────────────
    ProgressionVisualizerComponent m_progVisualizer;
    PianoRollPreviewComponent      m_pianoRoll;

    // ── Arp controls ──────────────────────────────────────────────────────
    LabeledComboBox m_rateBox      { "Note Rate" };
    LabeledComboBox m_directionBox { "Direction" };
    LabeledSlider   m_octaveSlider { "Octave Range" };
    LabeledSlider   m_gateSlider   { "Gate" };
    LabeledSlider   m_velSlider    { "Velocity" };
    juce::ToggleButton m_velRandBtn { "Velocity Randomise" };

    // APVTS attachments for arp sliders/combos
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_rateAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_dirAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   m_octaveAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   m_gateAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   m_velAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   m_velRandAttach;

    // ── Callbacks ─────────────────────────────────────────────────────────
    void onGenreChanged();
    void onProgressionChanged();
    void onKeyChanged();
    void onModeChanged();
    void onSuggest();

    void populateProgressionBox (int genreIndex);

    int m_currentGenreIndex      { 0 };
    int m_currentProgressionIdx  { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChordArpEditor)
};
