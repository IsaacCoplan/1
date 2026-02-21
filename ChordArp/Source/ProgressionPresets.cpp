#include "ProgressionPresets.h"

// ─────────────────────────────────────────────────────────────────────────────
// Static storage
// ─────────────────────────────────────────────────────────────────────────────
std::map<std::string, std::vector<Progression>> ProgressionPresets::s_data;
bool ProgressionPresets::s_built = false;

// ─────────────────────────────────────────────────────────────────────────────
// Helpers for building RomanChord entries concisely
// ─────────────────────────────────────────────────────────────────────────────
static RomanChord RC (int degree, int semitoneOffset,
                      ChordQuality quality, const std::string& label)
{
    return { degree, semitoneOffset, quality, label };
}

// Shorthands so the progression tables below are easy to read
//   I  = degree 1, offset 0
//   bII = degree 2, offset -1, etc.
static RomanChord I    (ChordQuality q, const std::string& l) { return RC(1,  0, q, l); }
static RomanChord bII  (ChordQuality q, const std::string& l) { return RC(2, -1, q, l); }
static RomanChord II   (ChordQuality q, const std::string& l) { return RC(2,  0, q, l); }
static RomanChord III  (ChordQuality q, const std::string& l) { return RC(3,  0, q, l); }
static RomanChord bIII (ChordQuality q, const std::string& l) { return RC(3, -1, q, l); }
static RomanChord IV   (ChordQuality q, const std::string& l) { return RC(4,  0, q, l); }
static RomanChord V    (ChordQuality q, const std::string& l) { return RC(5,  0, q, l); }
static RomanChord bVI  (ChordQuality q, const std::string& l) { return RC(6, -1, q, l); }
static RomanChord VI   (ChordQuality q, const std::string& l) { return RC(6,  0, q, l); }
static RomanChord bVII (ChordQuality q, const std::string& l) { return RC(7, -1, q, l); }
static RomanChord VII  (ChordQuality q, const std::string& l) { return RC(7,  0, q, l); }
// Lower-case helpers (same degree, minor quality — just for readability)
static RomanChord i    (ChordQuality q, const std::string& l) { return RC(1,  0, q, l); }
static RomanChord ii   (ChordQuality q, const std::string& l) { return RC(2,  0, q, l); }
static RomanChord iii  (ChordQuality q, const std::string& l) { return RC(3,  0, q, l); }
static RomanChord iv   (ChordQuality q, const std::string& l) { return RC(4,  0, q, l); }
static RomanChord v    (ChordQuality q, const std::string& l) { return RC(5,  0, q, l); }
static RomanChord vi   (ChordQuality q, const std::string& l) { return RC(6,  0, q, l); }
static RomanChord vii  (ChordQuality q, const std::string& l) { return RC(7,  0, q, l); }

using CQ = ChordQuality;

// ─────────────────────────────────────────────────────────────────────────────
void ProgressionPresets::buildAll()
{
    if (s_built) return;
    s_built = true;

    // ── Pop ───────────────────────────────────────────────────────────────
    {
        auto& v = s_data["Pop"];

        // "Axis"  I–V–vi–IV
        v.push_back({ "Axis", {
            I  (CQ::Major,  "I"),
            V  (CQ::Major,  "V"),
            vi (CQ::Minor,  "vi"),
            IV (CQ::Major,  "IV"),
        }});

        // "Three Chord"  I–IV–V–I
        v.push_back({ "Three Chord", {
            I  (CQ::Major, "I"),
            IV (CQ::Major, "IV"),
            V  (CQ::Major, "V"),
            I  (CQ::Major, "I"),
        }});

        // "Emotional Minor"  vi–IV–I–V
        v.push_back({ "Emotional Minor", {
            vi (CQ::Minor, "vi"),
            IV (CQ::Major, "IV"),
            I  (CQ::Major, "I"),
            V  (CQ::Major, "V"),
        }});

        // "Anthem"  I–V–IV–V
        v.push_back({ "Anthem", {
            I  (CQ::Major, "I"),
            V  (CQ::Major, "V"),
            IV (CQ::Major, "IV"),
            V  (CQ::Major, "V"),
        }});
    }

    // ── Jazz ──────────────────────────────────────────────────────────────
    {
        auto& v = s_data["Jazz"];

        // "ii–V–I"  iim7–V7–Imaj7
        v.push_back({ "ii-V-I", {
            ii  (CQ::Minor7,    "iim7"),
            V   (CQ::Dominant7, "V7"),
            I   (CQ::Major7,    "Imaj7"),
        }});

        // "Turnaround"  Imaj7–VI7–iim7–V7
        v.push_back({ "Turnaround", {
            I   (CQ::Major7,    "Imaj7"),
            VI  (CQ::Dominant7, "VI7"),
            ii  (CQ::Minor7,    "iim7"),
            V   (CQ::Dominant7, "V7"),
        }});

        // "Long Turnaround"  iii7–VI7–iim7–V7–Imaj7
        v.push_back({ "Long Turnaround", {
            iii (CQ::Minor7,    "iii7"),
            VI  (CQ::Dominant7, "VI7"),
            ii  (CQ::Minor7,    "iim7"),
            V   (CQ::Dominant7, "V7"),
            I   (CQ::Major7,    "Imaj7"),
        }});

        // "Rhythm Changes"  Imaj7–VI7–iim7–V7 (repeating)
        v.push_back({ "Rhythm Changes", {
            I   (CQ::Major7,    "Imaj7"),
            VI  (CQ::Dominant7, "VI7"),
            ii  (CQ::Minor7,    "iim7"),
            V   (CQ::Dominant7, "V7"),
        }});

        // "Modal Vamp"  Im7–bVIImaj7
        v.push_back({ "Modal Vamp", {
            i    (CQ::Minor7, "Im7"),
            bVII (CQ::Major7, "bVIImaj7"),
        }});
    }

    // ── Blues ─────────────────────────────────────────────────────────────
    {
        auto& v = s_data["Blues"];

        // Standard 12-bar:  I7 I7 I7 I7 | IV7 IV7 I7 I7 | V7 IV7 I7 V7
        v.push_back({ "12-Bar Blues", {
            I  (CQ::Dominant7, "I7"),
            I  (CQ::Dominant7, "I7"),
            I  (CQ::Dominant7, "I7"),
            I  (CQ::Dominant7, "I7"),
            IV (CQ::Dominant7, "IV7"),
            IV (CQ::Dominant7, "IV7"),
            I  (CQ::Dominant7, "I7"),
            I  (CQ::Dominant7, "I7"),
            V  (CQ::Dominant7, "V7"),
            IV (CQ::Dominant7, "IV7"),
            I  (CQ::Dominant7, "I7"),
            V  (CQ::Dominant7, "V7"),
        }});

        // Quick change: IV7 in bar 2
        v.push_back({ "Quick Change", {
            I  (CQ::Dominant7, "I7"),
            IV (CQ::Dominant7, "IV7"),
            I  (CQ::Dominant7, "I7"),
            I  (CQ::Dominant7, "I7"),
            IV (CQ::Dominant7, "IV7"),
            IV (CQ::Dominant7, "IV7"),
            I  (CQ::Dominant7, "I7"),
            I  (CQ::Dominant7, "I7"),
            V  (CQ::Dominant7, "V7"),
            IV (CQ::Dominant7, "IV7"),
            I  (CQ::Dominant7, "I7"),
            V  (CQ::Dominant7, "V7"),
        }});

        // Minor blues: im7 iv m7
        v.push_back({ "Minor Blues", {
            i  (CQ::Minor7,    "im7"),
            i  (CQ::Minor7,    "im7"),
            i  (CQ::Minor7,    "im7"),
            i  (CQ::Minor7,    "im7"),
            iv (CQ::Minor7,    "ivm7"),
            iv (CQ::Minor7,    "ivm7"),
            i  (CQ::Minor7,    "im7"),
            i  (CQ::Minor7,    "im7"),
            V  (CQ::Dominant7, "V7"),
            iv (CQ::Minor7,    "ivm7"),
            i  (CQ::Minor7,    "im7"),
            V  (CQ::Dominant7, "V7"),
        }});

        // Jazz blues
        v.push_back({ "Jazz Blues", {
            I  (CQ::Dominant7, "I7"),
            IV (CQ::Dominant7, "IV7"),
            I  (CQ::Dominant7, "I7"),
            I  (CQ::Dominant7, "I7"),
            IV (CQ::Dominant7, "IV7"),
            IV (CQ::Dominant7, "IV7"),
            I  (CQ::Dominant7, "I7"),
            VI (CQ::Dominant7, "VI7"),
            ii (CQ::Minor7,    "iim7"),
            V  (CQ::Dominant7, "V7"),
            I  (CQ::Dominant7, "I7"),
            V  (CQ::Dominant7, "V7"),
        }});
    }

    // ── Sad/Emotional ─────────────────────────────────────────────────────
    {
        auto& v = s_data["Sad"];

        // "Melancholic Loop"  i–VI–III–VII
        v.push_back({ "Melancholic Loop", {
            i   (CQ::Minor, "i"),
            bVI (CQ::Major, "bVI"),
            bIII(CQ::Major, "bIII"),
            bVII(CQ::Major, "bVII"),
        }});

        // "Tragic"  i–iv–i–V
        v.push_back({ "Tragic", {
            i  (CQ::Minor,     "i"),
            iv (CQ::Minor,     "iv"),
            i  (CQ::Minor,     "i"),
            V  (CQ::Dominant7, "V"),
        }});

        // "Unresolved"  i–VII–VI–VII
        v.push_back({ "Unresolved", {
            i   (CQ::Minor, "i"),
            bVII(CQ::Major, "bVII"),
            bVI (CQ::Major, "bVI"),
            bVII(CQ::Major, "bVII"),
        }});

        // "Brooding"  i–VI–VII–i
        v.push_back({ "Brooding", {
            i   (CQ::Minor, "i"),
            bVI (CQ::Major, "bVI"),
            bVII(CQ::Major, "bVII"),
            i   (CQ::Minor, "i"),
        }});
    }

    // ── R&B / Soul ────────────────────────────────────────────────────────
    {
        auto& v = s_data["R&B"];

        // "Smooth"  Imaj7–iii7–vi7–IV
        v.push_back({ "Smooth", {
            I   (CQ::Major7,    "Imaj7"),
            iii (CQ::Minor7,    "iii7"),
            vi  (CQ::Minor7,    "vi7"),
            IV  (CQ::Major,     "IV"),
        }});

        // "Neo-Soul Vamp"  IV–I
        v.push_back({ "Neo-Soul Vamp", {
            IV (CQ::Major7, "IVmaj7"),
            I  (CQ::Major7, "Imaj7"),
        }});

        // "Soul ii–V"  iim7–V7–Imaj7
        v.push_back({ "Soul ii-V", {
            ii (CQ::Minor7,    "iim7"),
            V  (CQ::Dominant7, "V7"),
            I  (CQ::Major7,    "Imaj7"),
        }});

        // "Minor Groove"  i7–bVII–bVI–bVII
        v.push_back({ "Minor Groove", {
            i   (CQ::Minor7, "i7"),
            bVII(CQ::Major,  "bVII"),
            bVI (CQ::Major,  "bVI"),
            bVII(CQ::Major,  "bVII"),
        }});
    }

    // ── Lo-Fi ─────────────────────────────────────────────────────────────
    {
        auto& v = s_data["Lo-Fi"];

        // "Classic Loop"  Imaj7–iii7–iim7–V7
        v.push_back({ "Classic Loop", {
            I   (CQ::Major7,    "Imaj7"),
            iii (CQ::Minor7,    "iii7"),
            ii  (CQ::Minor7,    "iim7"),
            V   (CQ::Dominant7, "V7"),
        }});

        // "Gentle"  vi7–IV–I–V
        v.push_back({ "Gentle", {
            vi (CQ::Minor7, "vi7"),
            IV (CQ::Major,  "IV"),
            I  (CQ::Major,  "I"),
            V  (CQ::Major,  "V"),
        }});

        // "Warm Cycle"  iim7–V7–Imaj7–vim7
        v.push_back({ "Warm Cycle", {
            ii (CQ::Minor7,    "iim7"),
            V  (CQ::Dominant7, "V7"),
            I  (CQ::Major7,    "Imaj7"),
            vi (CQ::Minor7,    "vim7"),
        }});
    }

    // ── Classical ─────────────────────────────────────────────────────────
    {
        auto& v = s_data["Classical"];

        // "Authentic Cadence"  I–IV–V–I
        v.push_back({ "Authentic Cadence", {
            I  (CQ::Major, "I"),
            IV (CQ::Major, "IV"),
            V  (CQ::Major, "V"),
            I  (CQ::Major, "I"),
        }});

        // "Standard Phrase"  I–ii–V–I
        v.push_back({ "Standard Phrase", {
            I  (CQ::Major, "I"),
            ii (CQ::Minor, "ii"),
            V  (CQ::Major, "V"),
            I  (CQ::Major, "I"),
        }});

        // "Circle of Fifths"  I–IV–vii°–iii–vi–ii–V–I
        v.push_back({ "Circle of Fifths", {
            I   (CQ::Major, "I"),
            IV  (CQ::Major, "IV"),
            vii (CQ::Dim,   "vii°"),
            iii (CQ::Minor, "iii"),
            vi  (CQ::Minor, "vi"),
            ii  (CQ::Minor, "ii"),
            V   (CQ::Major, "V"),
            I   (CQ::Major, "I"),
        }});

        // "Deceptive"  I–IV–V–vi–IV–V–I
        v.push_back({ "Deceptive", {
            I  (CQ::Major, "I"),
            IV (CQ::Major, "IV"),
            V  (CQ::Major, "V"),
            vi (CQ::Minor, "vi"),
            IV (CQ::Major, "IV"),
            V  (CQ::Major, "V"),
            I  (CQ::Major, "I"),
        }});
    }

    // ── Latin / Bossa Nova ────────────────────────────────────────────────
    {
        auto& v = s_data["Latin"];

        // "Tritone Vamp"  Imaj7–bIIImaj7
        v.push_back({ "Tritone Vamp", {
            I    (CQ::Major7, "Imaj7"),
            bIII (CQ::Major7, "bIIImaj7"),
        }});

        // "Minor ii–V–i"  iim7b5–V7–im
        v.push_back({ "Minor ii-V-i", {
            ii (CQ::HalfDim7,   "iim7b5"),
            V  (CQ::Dominant7,  "V7"),
            i  (CQ::Minor,      "im"),
        }});

        // "Bossa Cycle"  I–IV–iii–vi–ii–V
        v.push_back({ "Bossa Cycle", {
            I   (CQ::Major7,    "Imaj7"),
            IV  (CQ::Major7,    "IVmaj7"),
            iii (CQ::Minor7,    "iii7"),
            vi  (CQ::Minor7,    "vi7"),
            ii  (CQ::Minor7,    "iim7"),
            V   (CQ::Dominant7, "V7"),
        }});
    }
}

// ─────────────────────────────────────────────────────────────────────────────
const juce::StringArray& ProgressionPresets::getGenreNames()
{
    static juce::StringArray names {
        "Pop", "Jazz", "Blues", "Sad", "R&B", "Lo-Fi", "Classical", "Latin"
    };
    return names;
}

const std::vector<Progression>& ProgressionPresets::getProgressions (int genreIndex)
{
    buildAll();
    const auto& names = getGenreNames();
    auto it = s_data.find (names[genreIndex].toStdString());
    static std::vector<Progression> empty;
    if (it == s_data.end()) return empty;
    return it->second;
}

const std::vector<Progression>* ProgressionPresets::getProgressionsByName (const juce::String& genre)
{
    buildAll();
    auto it = s_data.find (genre.toStdString());
    if (it == s_data.end()) return nullptr;
    return &it->second;
}

const Progression& ProgressionPresets::suggest (int genreIndex, int& suggestionIndex)
{
    const auto& progs = getProgressions (genreIndex);
    if (progs.empty())
    {
        // Fallback — return first Pop progression
        return getProgressions (0)[0];
    }
    suggestionIndex = (suggestionIndex + 1) % static_cast<int> (progs.size());
    return progs[static_cast<size_t> (suggestionIndex)];
}
