#pragma once
#include <JuceHeader.h>
#include <vector>
#include <string>
#include <map>

// ─────────────────────────────────────────────────────────────────────────────
// Roman-numeral chord descriptor
// Encodes everything needed to build a chord in any key/mode at runtime.
// ─────────────────────────────────────────────────────────────────────────────

enum class ChordQuality
{
    Major,          // triad
    Minor,          // triad
    Dominant7,      // dom7
    Major7,         // maj7
    Minor7,         // min7
    HalfDim7,       // m7b5
    Dim7,           // dim7
    Augmented,      // aug
    Sus4,
    MinorMajor7,    // mM7
    Dim,            // dim triad
};

struct RomanChord
{
    int  scaleDegree;       // 1-based (1=I, 2=II, …)
    int  semitoneOffset;    // additional semitone shift from the scale degree root
                            // (e.g. bVII = degree 7, offset -1)
    ChordQuality quality;
    std::string label;      // display label like "iim7", "V7", "Imaj7"
};

struct Progression
{
    std::string name;
    std::vector<RomanChord> chords;
};

// ─────────────────────────────────────────────────────────────────────────────
class ProgressionPresets
{
public:
    // Genre names exactly as shown in the UI dropdown
    static const juce::StringArray& getGenreNames();

    // All progressions for a genre (by index matching getGenreNames())
    static const std::vector<Progression>& getProgressions (int genreIndex);

    // Convenience: look up by genre name
    static const std::vector<Progression>* getProgressionsByName (const juce::String& genre);

    // AI-suggest: cycles through progressions for the given genre, returning the
    // next one each call (stateless — caller passes the current suggestion index).
    static const Progression& suggest (int genreIndex, int& suggestionIndex);

private:
    static void buildAll();
    static std::map<std::string, std::vector<Progression>> s_data;
    static bool s_built;
};
