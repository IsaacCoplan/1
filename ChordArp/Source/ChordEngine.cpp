#include "ChordEngine.h"
#include <cassert>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Scale interval tables (semitones from root, 7 degrees)
// ─────────────────────────────────────────────────────────────────────────────
const std::vector<int>& ChordEngine::modeIntervals (ScaleMode m)
{
    // Each entry: semitone offset of scale degrees 1–7 from the root
    static const std::vector<int> ionian       { 0, 2, 4, 5, 7, 9, 11 };
    static const std::vector<int> dorian       { 0, 2, 3, 5, 7, 9, 10 };
    static const std::vector<int> phrygian     { 0, 1, 3, 5, 7, 8, 10 };
    static const std::vector<int> lydian       { 0, 2, 4, 6, 7, 9, 11 };
    static const std::vector<int> mixolydian   { 0, 2, 4, 5, 7, 9, 10 };
    static const std::vector<int> aeolian      { 0, 2, 3, 5, 7, 8, 10 };
    static const std::vector<int> locrian      { 0, 1, 3, 5, 6, 8, 10 };
    static const std::vector<int> harmMinor    { 0, 2, 3, 5, 7, 8, 11 };
    static const std::vector<int> melodMinor   { 0, 2, 3, 5, 7, 9, 11 };
    // Pentatonic: use nearest 7 degrees (pad with extra for degree access)
    static const std::vector<int> pentMajor    { 0, 2, 4, 7, 9, 12, 14 };
    static const std::vector<int> pentMinor    { 0, 3, 5, 7, 10, 12, 15 };
    static const std::vector<int> blues        { 0, 3, 5, 6, 7, 10, 12 };

    switch (m)
    {
        case ScaleMode::Ionian:          return ionian;
        case ScaleMode::Dorian:          return dorian;
        case ScaleMode::Phrygian:        return phrygian;
        case ScaleMode::Lydian:          return lydian;
        case ScaleMode::Mixolydian:      return mixolydian;
        case ScaleMode::Aeolian:         return aeolian;
        case ScaleMode::Locrian:         return locrian;
        case ScaleMode::HarmonicMinor:   return harmMinor;
        case ScaleMode::MelodicMinor:    return melodMinor;
        case ScaleMode::PentatonicMajor: return pentMajor;
        case ScaleMode::PentatonicMinor: return pentMinor;
        case ScaleMode::Blues:           return blues;
        default:                         return ionian;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Build MIDI note numbers from a root semitone + quality
// baseOctave: e.g. 4 → C4 = MIDI 60
// ─────────────────────────────────────────────────────────────────────────────
std::vector<int> ChordEngine::buildChordNotes (int rootSemitone,
                                               ChordQuality quality,
                                               int baseOctave)
{
    // rootSemitone is 0-based within an octave; map to absolute MIDI
    int root = (baseOctave + 1) * 12 + rootSemitone;

    std::vector<int> notes;
    notes.push_back (root);

    switch (quality)
    {
        // ── Triads ──────────────────────────────────────────────────────
        case ChordQuality::Major:
            notes.push_back (root + 4);   // major 3rd
            notes.push_back (root + 7);   // perfect 5th
            break;
        case ChordQuality::Minor:
            notes.push_back (root + 3);   // minor 3rd
            notes.push_back (root + 7);
            break;
        case ChordQuality::Dim:
            notes.push_back (root + 3);
            notes.push_back (root + 6);   // diminished 5th
            break;
        case ChordQuality::Augmented:
            notes.push_back (root + 4);
            notes.push_back (root + 8);   // augmented 5th
            break;
        case ChordQuality::Sus4:
            notes.push_back (root + 5);   // perfect 4th
            notes.push_back (root + 7);
            break;

        // ── 7th chords ──────────────────────────────────────────────────
        case ChordQuality::Dominant7:
            notes.push_back (root + 4);
            notes.push_back (root + 7);
            notes.push_back (root + 10);  // minor 7th
            break;
        case ChordQuality::Major7:
            notes.push_back (root + 4);
            notes.push_back (root + 7);
            notes.push_back (root + 11);  // major 7th
            break;
        case ChordQuality::Minor7:
            notes.push_back (root + 3);
            notes.push_back (root + 7);
            notes.push_back (root + 10);
            break;
        case ChordQuality::HalfDim7:      // m7b5
            notes.push_back (root + 3);
            notes.push_back (root + 6);
            notes.push_back (root + 10);
            break;
        case ChordQuality::Dim7:
            notes.push_back (root + 3);
            notes.push_back (root + 6);
            notes.push_back (root + 9);   // diminished 7th (bb7)
            break;
        case ChordQuality::MinorMajor7:
            notes.push_back (root + 3);
            notes.push_back (root + 7);
            notes.push_back (root + 11);
            break;
        default:
            notes.push_back (root + 4);
            notes.push_back (root + 7);
            break;
    }

    return notes;
}

// ─────────────────────────────────────────────────────────────────────────────
// Quality suffix strings for display
// ─────────────────────────────────────────────────────────────────────────────
std::string ChordEngine::chordDisplayName (int rootSemitone, ChordQuality quality)
{
    // Note name (sharp spelling)
    static const char* noteNames[] = {
        "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
    };
    std::string name = noteNames[rootSemitone % 12];

    switch (quality)
    {
        case ChordQuality::Major:        break;                 // "C"
        case ChordQuality::Minor:        name += "m";   break;
        case ChordQuality::Dominant7:    name += "7";   break;
        case ChordQuality::Major7:       name += "maj7"; break;
        case ChordQuality::Minor7:       name += "m7";  break;
        case ChordQuality::HalfDim7:     name += "m7b5"; break;
        case ChordQuality::Dim7:         name += "dim7"; break;
        case ChordQuality::Dim:          name += "dim";  break;
        case ChordQuality::Augmented:    name += "aug";  break;
        case ChordQuality::Sus4:         name += "sus4"; break;
        case ChordQuality::MinorMajor7:  name += "mM7";  break;
        default:                         break;
    }
    return name;
}

// ─────────────────────────────────────────────────────────────────────────────
ChordEngine::ChordEngine() = default;

void ChordEngine::setKey  (int rootMidiPitch, ScaleMode mode)
{
    m_root = rootMidiPitch % 12;
    m_mode = mode;
}

void ChordEngine::setMode (ScaleMode mode) { m_mode = mode; }
void ChordEngine::setRoot (int rootMidiPitch) { m_root = rootMidiPitch % 12; }

// ─────────────────────────────────────────────────────────────────────────────
ResolvedChord ChordEngine::resolve (const RomanChord& rc, int baseOctave) const
{
    const auto& intervals = modeIntervals (m_mode);

    // Scale degree is 1-based; clamp to available degrees
    int degreeIdx = juce::jlimit (0, (int)intervals.size() - 1, rc.scaleDegree - 1);
    int scaleDegreeOffset = intervals[static_cast<size_t>(degreeIdx)];

    // Absolute semitone within chromatic octave
    int rootSemitone = (m_root + scaleDegreeOffset + rc.semitoneOffset + 120) % 12;

    ResolvedChord chord;
    chord.midiNotes  = buildChordNotes (rootSemitone, rc.quality, baseOctave);
    chord.displayName = chordDisplayName (rootSemitone, rc.quality);
    return chord;
}

// ─────────────────────────────────────────────────────────────────────────────
std::vector<ResolvedChord> ChordEngine::resolveProgression (const Progression& prog,
                                                             int baseOctave) const
{
    std::vector<ResolvedChord> result;
    result.reserve (prog.chords.size());
    for (const auto& rc : prog.chords)
        result.push_back (resolve (rc, baseOctave));
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
std::vector<ResolvedChord> ChordEngine::getDiatonicChords (int baseOctave) const
{
    // Build default qualities for each degree in the mode (Major scale quality set)
    const auto& intervals = modeIntervals (m_mode);
    std::vector<ResolvedChord> diatonic;

    // Quality assignment for a Major scale by degree: I ii iii IV V vi vii°
    static const ChordQuality majorQualities[] = {
        ChordQuality::Major, ChordQuality::Minor, ChordQuality::Minor,
        ChordQuality::Major, ChordQuality::Major, ChordQuality::Minor,
        ChordQuality::Dim
    };
    // For minor (Aeolian): i ii° III iv v VI VII
    static const ChordQuality minorQualities[] = {
        ChordQuality::Minor, ChordQuality::Dim,   ChordQuality::Major,
        ChordQuality::Minor, ChordQuality::Minor, ChordQuality::Major,
        ChordQuality::Major
    };

    const ChordQuality* qualities = (m_mode == ScaleMode::Aeolian ||
                                     m_mode == ScaleMode::Dorian  ||
                                     m_mode == ScaleMode::Phrygian ||
                                     m_mode == ScaleMode::Locrian  ||
                                     m_mode == ScaleMode::HarmonicMinor)
                                        ? minorQualities
                                        : majorQualities;

    int numDegrees = (int)std::min (intervals.size(), (size_t)7u);
    for (int i = 0; i < numDegrees; ++i)
    {
        int rootSemitone = (m_root + intervals[static_cast<size_t>(i)]) % 12;
        ResolvedChord chord;
        chord.midiNotes  = buildChordNotes (rootSemitone, qualities[i], baseOctave);
        chord.displayName = chordDisplayName (rootSemitone, qualities[i]);
        diatonic.push_back (chord);
    }
    return diatonic;
}

// ─────────────────────────────────────────────────────────────────────────────
juce::StringArray ChordEngine::getModeNames()
{
    return {
        "Major (Ionian)", "Dorian", "Phrygian", "Lydian",
        "Mixolydian", "Natural Minor (Aeolian)", "Locrian",
        "Harmonic Minor", "Melodic Minor",
        "Pentatonic Major", "Pentatonic Minor", "Blues"
    };
}

juce::String ChordEngine::noteName (int semitone)
{
    static const char* names[] = {
        "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
    };
    return names[semitone % 12];
}
