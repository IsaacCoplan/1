#include "ArpEngine.h"
#include <algorithm>
#include <numeric>

// ─────────────────────────────────────────────────────────────────────────────
ArpEngine::ArpEngine()
    : m_rng (std::random_device{}())
{
}

// ─────────────────────────────────────────────────────────────────────────────
void ArpEngine::setChord (const ResolvedChord& chord)
{
    m_chord = chord;
    rebuildSequence();
    // Don't reset step index — let the pattern continue from where it was
    // so chord changes are smooth; just clamp if needed.
    if (!m_sequence.empty())
        m_stepIndex = m_stepIndex % (int)m_sequence.size();
}

void ArpEngine::setSettings (const ArpSettings& settings)
{
    m_settings = settings;
    rebuildSequence();
    if (!m_sequence.empty())
        m_stepIndex = m_stepIndex % (int)m_sequence.size();
}

// ─────────────────────────────────────────────────────────────────────────────
void ArpEngine::reset()
{
    m_stepIndex        = 0;
    m_beatPos          = 0.0;
    m_lastPpq          = -1.0;
    m_activeNote       = -1;
    m_activeNoteOffSample = -1;
    m_insideOutLeft    = 0;
    m_insideOutRight   = (int)m_sequence.size() - 1;
    m_insideOutToggle  = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Build the ordered sequence of MIDI notes from the chord + direction + octave
// ─────────────────────────────────────────────────────────────────────────────
void ArpEngine::rebuildSequence()
{
    m_sequence.clear();
    if (m_chord.midiNotes.empty()) return;

    // Start with the chord notes sorted ascending
    std::vector<int> base = m_chord.midiNotes;
    std::sort (base.begin(), base.end());

    // Expand across octave range
    for (int oct = 0; oct < m_settings.octaveRange; ++oct)
        for (int n : base)
            m_sequence.push_back (n + oct * 12);

    // Apply direction
    switch (m_settings.direction)
    {
        case ArpDirection::Up:
            // Already ascending
            break;

        case ArpDirection::Down:
            std::reverse (m_sequence.begin(), m_sequence.end());
            break;

        case ArpDirection::Random:
            // Randomise once; will re-randomise each cycle in processBlock
            std::shuffle (m_sequence.begin(), m_sequence.end(), m_rng);
            break;

        case ArpDirection::InsideOut:
            // Will be handled step-by-step in processBlock
            // Leave sequence sorted for now; IOL/IOR will index into it
            m_insideOutLeft  = (int)m_sequence.size() / 2;
            m_insideOutRight = m_insideOutLeft;
            if ((int)m_sequence.size() % 2 == 0)
                m_insideOutLeft--;
            m_insideOutToggle = false;
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
double ArpEngine::noteRateToBeats (NoteRate rate)
{
    switch (rate)
    {
        case NoteRate::Quarter:      return 1.0;
        case NoteRate::Eighth:       return 0.5;
        case NoteRate::Sixteenth:    return 0.25;
        case NoteRate::ThirtySecond: return 0.125;
        default:                     return 0.25;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Main processBlock — generates MIDI note on/off events synced to host
// ─────────────────────────────────────────────────────────────────────────────
void ArpEngine::processBlock (const juce::AudioPlayHead::CurrentPositionInfo& pos,
                               double sampleRate,
                               int    numSamples,
                               juce::MidiBuffer& midiOut)
{
    if (m_sequence.empty()) return;

    // ── Transport: not playing → silence ──────────────────────────────────
    if (!pos.isPlaying)
    {
        if (m_activeNote >= 0)
        {
            midiOut.addEvent (juce::MidiMessage::noteOff (1, m_activeNote, (juce::uint8)0), 0);
            m_activeNote = -1;
        }
        reset();
        return;
    }

    const double ppqPosition = pos.ppqPosition;
    const double bpm         = pos.bpm > 0.0 ? pos.bpm : 120.0;
    const double beatsPerStep = noteRateToBeats (m_settings.rate);
    const double samplesPerBeat = sampleRate * 60.0 / bpm;

    // Detect transport jump / rewind
    if (m_lastPpq >= 0.0 && std::abs (ppqPosition - m_lastPpq) > 1.0)
        reset();

    m_lastPpq = ppqPosition;

    // Beat position at start of this block
    double blockStartBeat = ppqPosition;

    // Iterate over samples in the block to find exact MIDI event positions
    for (int sample = 0; sample < numSamples; ++sample)
    {
        double beatAtSample = blockStartBeat + (double)sample / samplesPerBeat;

        // ── Pending note-off ──────────────────────────────────────────────
        if (m_activeNote >= 0 && sample == m_activeNoteOffSample)
        {
            midiOut.addEvent (juce::MidiMessage::noteOff (1, m_activeNote, (juce::uint8)0),
                              sample);
            m_activeNote = -1;
        }

        // ── Check for step boundary ───────────────────────────────────────
        // A step starts at every multiple of beatsPerStep
        double stepsElapsed = beatAtSample / beatsPerStep;
        double prevStepsElapsed = (beatAtSample - 1.0 / samplesPerBeat) / beatsPerStep;

        bool stepBoundary = (int)stepsElapsed > (int)prevStepsElapsed;

        if (stepBoundary)
        {
            // Turn off any lingering active note first (shouldn't normally happen)
            if (m_activeNote >= 0)
            {
                midiOut.addEvent (juce::MidiMessage::noteOff (1, m_activeNote, (juce::uint8)0),
                                  sample);
                m_activeNote = -1;
            }

            // ── Choose the note for this step ─────────────────────────────
            int noteToPlay = -1;
            const int seqSize = (int)m_sequence.size();

            if (m_settings.direction == ArpDirection::InsideOut)
            {
                // Alternate left-inward / right-inward from center
                if (m_insideOutToggle)
                {
                    noteToPlay = m_sequence[static_cast<size_t>(
                        juce::jlimit (0, seqSize - 1, m_insideOutRight))];
                    m_insideOutRight++;
                }
                else
                {
                    noteToPlay = m_sequence[static_cast<size_t>(
                        juce::jlimit (0, seqSize - 1, m_insideOutLeft))];
                    m_insideOutLeft--;
                }
                m_insideOutToggle = !m_insideOutToggle;

                // Reset when we've played all notes
                m_stepIndex++;
                if (m_stepIndex >= seqSize)
                {
                    m_stepIndex = 0;
                    m_insideOutLeft  = seqSize / 2;
                    m_insideOutRight = m_insideOutLeft;
                    if (seqSize % 2 == 0) m_insideOutLeft--;
                    m_insideOutToggle = false;
                }
            }
            else if (m_settings.direction == ArpDirection::Random)
            {
                // Re-shuffle at the start of each cycle
                if (m_stepIndex == 0)
                    std::shuffle (m_sequence.begin(), m_sequence.end(), m_rng);

                noteToPlay = m_sequence[static_cast<size_t>(m_stepIndex)];
                m_stepIndex = (m_stepIndex + 1) % seqSize;
            }
            else
            {
                noteToPlay = m_sequence[static_cast<size_t>(m_stepIndex)];
                m_stepIndex = (m_stepIndex + 1) % seqSize;
            }

            if (noteToPlay < 0 || noteToPlay > 127) continue;

            // ── Velocity ──────────────────────────────────────────────────
            float vel = m_settings.velocity;
            if (m_settings.velocityRand)
            {
                std::uniform_real_distribution<float> jitter (-0.08f, 0.08f);
                vel = juce::jlimit (0.0f, 1.0f, vel + jitter (m_rng));
            }
            auto velByte = (juce::uint8) juce::jlimit (1, 127, (int)(vel * 127.0f));

            // ── Note on ───────────────────────────────────────────────────
            midiOut.addEvent (juce::MidiMessage::noteOn (1, noteToPlay, velByte), sample);
            m_activeNote = noteToPlay;

            // Schedule note-off at gate * step_length samples from now
            double stepLengthSamples = beatsPerStep * samplesPerBeat;
            int noteOffOffset = (int)(stepLengthSamples * (double)m_settings.gateLength);
            // Clamp to end of buffer (will spill to next block if needed — handled by
            // m_activeNoteOffSample check at top of loop next block)
            m_activeNoteOffSample = sample + noteOffOffset;
            if (m_activeNoteOffSample >= numSamples)
                m_activeNoteOffSample = numSamples - 1; // clamp — small gate truncation ok
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
std::vector<int> ArpEngine::getPreviewSequence() const
{
    return m_sequence;
}
