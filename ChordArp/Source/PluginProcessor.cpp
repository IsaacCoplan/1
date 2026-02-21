#include "PluginProcessor.h"
#include "PluginEditor.h"

// ─────────────────────────────────────────────────────────────────────────────
// Parameter IDs
// ─────────────────────────────────────────────────────────────────────────────
static const juce::String kParamRate          = "rate";
static const juce::String kParamDirection     = "direction";
static const juce::String kParamOctaveRange   = "octaveRange";
static const juce::String kParamGate          = "gate";
static const juce::String kParamVelocity      = "velocity";
static const juce::String kParamVelRand       = "velocityRand";

// ─────────────────────────────────────────────────────────────────────────────
juce::AudioProcessorValueTreeState::ParameterLayout
ChordArpProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Note rate: 0=Quarter, 1=Eighth, 2=Sixteenth, 3=ThirtySecond
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { kParamRate, 1 }, "Note Rate",
        juce::StringArray { "1/4", "1/8", "1/16", "1/32" }, 2));

    // Direction: 0=Up, 1=Down, 2=Random, 3=InsideOut
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { kParamDirection, 1 }, "Direction",
        juce::StringArray { "Up", "Down", "Random", "Inside-Out" }, 0));

    // Octave range 1-3
    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { kParamOctaveRange, 1 }, "Octave Range", 1, 3, 1));

    // Gate length 0.05 – 1.0
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kParamGate, 1 }, "Gate Length",
        juce::NormalisableRange<float> (0.05f, 1.0f, 0.01f), 0.8f));

    // Velocity 0.0 – 1.0
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kParamVelocity, 1 }, "Velocity",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.75f));

    // Velocity randomize
    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { kParamVelRand, 1 }, "Velocity Randomise", false));

    return layout;
}

// ─────────────────────────────────────────────────────────────────────────────
ChordArpProcessor::ChordArpProcessor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      m_apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // Load default progression: Pop / Axis
    loadProgression (0, 0);
}

ChordArpProcessor::~ChordArpProcessor() = default;

// ─────────────────────────────────────────────────────────────────────────────
bool ChordArpProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // MIDI-only plugin — we don't care about audio buses, but JUCE requires
    // at least one enabled audio output for the standalone wrapper.
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled();
}

// ─────────────────────────────────────────────────────────────────────────────
void ChordArpProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    m_sampleRate = sampleRate;
    m_arpEngine.reset();
}

void ChordArpProcessor::releaseResources() {}

// ─────────────────────────────────────────────────────────────────────────────
void ChordArpProcessor::syncArpSettings()
{
    ArpSettings s;

    int rateIdx = (int) m_apvts.getRawParameterValue (kParamRate)->load();
    switch (rateIdx)
    {
        case 0: s.rate = NoteRate::Quarter;      break;
        case 1: s.rate = NoteRate::Eighth;       break;
        case 2: s.rate = NoteRate::Sixteenth;    break;
        case 3: s.rate = NoteRate::ThirtySecond; break;
        default: s.rate = NoteRate::Sixteenth;   break;
    }

    int dirIdx = (int) m_apvts.getRawParameterValue (kParamDirection)->load();
    switch (dirIdx)
    {
        case 0: s.direction = ArpDirection::Up;        break;
        case 1: s.direction = ArpDirection::Down;      break;
        case 2: s.direction = ArpDirection::Random;    break;
        case 3: s.direction = ArpDirection::InsideOut; break;
        default: s.direction = ArpDirection::Up;       break;
    }

    s.octaveRange  = (int)   m_apvts.getRawParameterValue (kParamOctaveRange)->load();
    s.gateLength   = (float) m_apvts.getRawParameterValue (kParamGate)->load();
    s.velocity     = (float) m_apvts.getRawParameterValue (kParamVelocity)->load();
    s.velocityRand = m_apvts.getRawParameterValue (kParamVelRand)->load() > 0.5f;

    m_arpEngine.setSettings (s);
}

void ChordArpProcessor::syncChordToArp()
{
    if (m_resolvedProg.empty()) return;
    int idx = juce::jlimit (0, (int)m_resolvedProg.size() - 1, m_chordIndex);
    m_arpEngine.setChord (m_resolvedProg[static_cast<size_t>(idx)]);
}

// ─────────────────────────────────────────────────────────────────────────────
void ChordArpProcessor::processBlock (juce::AudioBuffer<float>& audio,
                                      juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDeNorm;

    // Clear audio (MIDI only plugin)
    audio.clear();
    midi.clear();

    // Sync arp settings from parameters
    syncArpSettings();

    // Get transport info
    juce::AudioPlayHead::CurrentPositionInfo pos;
    if (auto* ph = getPlayHead())
        ph->getCurrentPosition (pos);
    else
    {
        pos.isPlaying   = false;
        pos.ppqPosition = 0.0;
        pos.bpm         = 120.0;
    }

    // ── Detect transport stop / reset ──────────────────────────────────────
    if (!pos.isPlaying && m_wasPlaying)
        m_arpEngine.reset();
    m_wasPlaying = pos.isPlaying;

    if (!pos.isPlaying) return;

    // ── Chord advance: advance to next chord every m_beatsPerChord ─────────
    {
        double ppq = pos.ppqPosition;

        // Detect jump
        if (m_lastPpqForChord >= 0.0 && std::abs (ppq - m_lastPpqForChord) > 1.0)
            m_chordBeatAccum = 0.0;

        if (m_lastPpqForChord >= 0.0)
        {
            double delta = ppq - m_lastPpqForChord;
            if (delta > 0.0)
            {
                m_chordBeatAccum += delta;
                if (m_chordBeatAccum >= m_beatsPerChord)
                {
                    m_chordBeatAccum = std::fmod (m_chordBeatAccum, m_beatsPerChord);
                    if (!m_resolvedProg.empty())
                    {
                        m_chordIndex = (m_chordIndex + 1) % (int)m_resolvedProg.size();
                        syncChordToArp();
                    }
                }
            }
        }
        m_lastPpqForChord = ppq;
    }

    // ── Generate arpeggio MIDI ──────────────────────────────────────────────
    m_arpEngine.processBlock (pos, m_sampleRate, audio.getNumSamples(), midi);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API called from editor
// ─────────────────────────────────────────────────────────────────────────────
void ChordArpProcessor::loadProgression (int genreIndex, int progressionIndex)
{
    const auto& progs = ProgressionPresets::getProgressions (genreIndex);
    if (progs.empty()) return;

    int idx = juce::jlimit (0, (int)progs.size() - 1, progressionIndex);
    m_currentProg  = progs[static_cast<size_t>(idx)];
    m_resolvedProg = m_chordEngine.resolveProgression (m_currentProg);
    m_chordIndex   = 0;
    m_chordBeatAccum = 0.0;
    syncChordToArp();
}

void ChordArpProcessor::setKey (int root)
{
    m_chordEngine.setRoot (root);
    m_resolvedProg = m_chordEngine.resolveProgression (m_currentProg);
    syncChordToArp();
}

void ChordArpProcessor::setMode (ScaleMode mode)
{
    m_chordEngine.setMode (mode);
    m_resolvedProg = m_chordEngine.resolveProgression (m_currentProg);
    syncChordToArp();
}

void ChordArpProcessor::suggestProgression (int genreIndex)
{
    m_currentProg  = ProgressionPresets::suggest (genreIndex, m_suggestionIdx);
    m_resolvedProg = m_chordEngine.resolveProgression (m_currentProg);
    m_chordIndex   = 0;
    m_chordBeatAccum = 0.0;
    syncChordToArp();
}

// ─────────────────────────────────────────────────────────────────────────────
juce::AudioProcessorEditor* ChordArpProcessor::createEditor()
{
    return new ChordArpEditor (*this);
}

// ─────────────────────────────────────────────────────────────────────────────
// State persistence
// ─────────────────────────────────────────────────────────────────────────────
void ChordArpProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = m_apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ChordArpProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml && xml->hasTagName (m_apvts.state.getType()))
        m_apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// ─────────────────────────────────────────────────────────────────────────────
// JUCE plugin entry point
// ─────────────────────────────────────────────────────────────────────────────
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChordArpProcessor();
}
