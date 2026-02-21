#pragma once
#include <JuceHeader.h>
#include "ProgressionPresets.h"
#include <vector>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// Mode definitions — semitone intervals from root for each scale degree
// ─────────────────────────────────────────────────────────────────────────────
enum class ScaleMode
{
    Ionian,       // Major
    Dorian,
    Phrygian,
    Lydian,
    Mixolydian,
    Aeolian,      // Natural Minor
    Locrian,
    HarmonicMinor,
    MelodicMinor,
    PentatonicMajor,
    PentatonicMinor,
    Blues,
};

// A resolved chord: MIDI note numbers for every note
struct ResolvedChord
{
    std::string displayName;          // e.g. "Cmaj7", "Am7", "F#7"
    std::vector<int> midiNotes;       // sorted ascending, octave 4 base
};

// ─────────────────────────────────────────────────────────────────────────────
class ChordEngine
{
public:
    ChordEngine();

    // Set the tonic (0=C … 11=B) and mode
    void setKey  (int rootMidiPitch, ScaleMode mode);
    void setMode (ScaleMode mode);
    void setRoot (int rootMidiPitch);

    int       getRoot() const { return m_root; }
    ScaleMode getMode() const { return m_mode; }

    // Resolve a RomanChord descriptor into actual MIDI notes
    // baseOctave: MIDI octave for root of chord (default 4 → C4=60)
    ResolvedChord resolve (const RomanChord& rc, int baseOctave = 4) const;

    // Resolve an entire progression
    std::vector<ResolvedChord> resolveProgression (const Progression& prog,
                                                   int baseOctave = 4) const;

    // Build diatonic chord options for current key/mode (7 chords)
    std::vector<ResolvedChord> getDiatonicChords (int baseOctave = 4) const;

    // Human-readable names
    static juce::StringArray getModeNames();
    static juce::String      noteName (int semitone); // 0=C … 11=B

private:
    int       m_root { 0 };    // 0=C
    ScaleMode m_mode { ScaleMode::Ionian };

    // Returns the semitone intervals for each scale degree (7 values)
    static const std::vector<int>& modeIntervals (ScaleMode m);

    // Build notes for a given root (absolute semitone) and quality
    static std::vector<int> buildChordNotes (int rootSemitone,
                                             ChordQuality quality,
                                             int baseOctave);

    // Display name: note name + quality suffix
    static std::string chordDisplayName (int rootSemitone, ChordQuality quality);
};
