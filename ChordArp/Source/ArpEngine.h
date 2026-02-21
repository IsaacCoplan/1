#pragma once
#include <JuceHeader.h>
#include "ChordEngine.h"
#include <vector>
#include <random>

// ─────────────────────────────────────────────────────────────────────────────
enum class ArpDirection { Up, Down, Random, InsideOut };
enum class NoteRate     { Quarter, Eighth, Sixteenth, ThirtySecond };

struct ArpSettings
{
    NoteRate     rate          { NoteRate::Sixteenth };
    ArpDirection direction     { ArpDirection::Up };
    int          octaveRange   { 1 };     // 1, 2, or 3 extra octaves
    float        gateLength    { 0.8f };  // 0.0–1.0 fraction of note duration
    float        velocity      { 0.75f }; // 0.0–1.0
    bool         velocityRand  { false }; // subtle velocity randomisation
};

// ─────────────────────────────────────────────────────────────────────────────
// ArpEngine: stateful arpeggiator that runs inside processBlock.
//
// Usage each processBlock call:
//   1. Call setChord()   if the active chord changed.
//   2. Call setSettings() if UI controls changed.
//   3. Call processBlock() — it fills the provided MidiBuffer.
// ─────────────────────────────────────────────────────────────────────────────
class ArpEngine
{
public:
    ArpEngine();

    void setChord    (const ResolvedChord& chord);
    void setSettings (const ArpSettings& settings);

    // Called every processBlock.
    // posInfo:       current host transport info
    // sampleRate:    current sample rate
    // numSamples:    block size in samples
    // midiOut:       buffer to write generated MIDI into
    void processBlock (const juce::AudioPlayHead::CurrentPositionInfo& posInfo,
                       double sampleRate,
                       int    numSamples,
                       juce::MidiBuffer& midiOut);

    // Reset arpeggiator state (e.g. on transport stop/rewind)
    void reset();

    // Returns the ordered note sequence for the current chord+settings
    // (used by the piano-roll preview in the editor).
    std::vector<int> getPreviewSequence() const;

    int getCurrentStepIndex() const { return m_stepIndex; }

private:
    // Build the note sequence from the current chord, direction, and octave range
    void rebuildSequence();

    // Convert NoteRate to quarter-note fractions (e.g. Sixteenth = 0.25)
    static double noteRateToBeats (NoteRate rate);

    ResolvedChord m_chord;
    ArpSettings   m_settings;

    std::vector<int> m_sequence;   // MIDI note numbers in playback order
    int              m_stepIndex { 0 };

    // Fractional beat position within the current arp step
    double m_beatPos  { 0.0 };
    double m_lastPpq  { -1.0 };   // detect transport jumps

    int    m_activeNote { -1 };   // currently sounding note (for note-off)
    int    m_activeNoteOffSample { -1 };

    // Inside-out iteration state
    int    m_insideOutLeft  { 0 };
    int    m_insideOutRight { 0 };
    bool   m_insideOutToggle { false };

    std::mt19937 m_rng;
};
