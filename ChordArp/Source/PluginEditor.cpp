#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "ProgressionPresets.h"
#include "ChordEngine.h"

// ─────────────────────────────────────────────────────────────────────────────
// Palette
// ─────────────────────────────────────────────────────────────────────────────
namespace Pal
{
    const juce::Colour bg         { 0xff1a1a2e };
    const juce::Colour panel      { 0xff16213e };
    const juce::Colour accent     { 0xff0f3460 };
    const juce::Colour highlight  { 0xffe94560 };
    const juce::Colour text       { 0xfff0f0f0 };
    const juce::Colour textDim    { 0xff9090a0 };
    const juce::Colour chordBlock { 0xff0f3460 };
    const juce::Colour activeBlock{ 0xffe94560 };
    const juce::Colour pianoWhite { 0xff2a2a4a };
    const juce::Colour pianoBlack { 0xff111122 };
    const juce::Colour noteOn     { 0xffe94560 };
}

// =============================================================================
// ChordBlockComponent
// =============================================================================
ChordBlockComponent::ChordBlockComponent()
{
    setInterceptsMouseClicks (false, false);
}

void ChordBlockComponent::setLabel (const juce::String& text) { m_label = text; repaint(); }
void ChordBlockComponent::setActive (bool active)             { m_active = active; repaint(); }

void ChordBlockComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced (3);
    g.setColour (m_active ? Pal::activeBlock : Pal::chordBlock);
    g.fillRoundedRectangle (bounds.toFloat(), 6.0f);

    g.setColour (m_active ? juce::Colours::white : Pal::textDim);
    g.setFont (juce::Font ("Verdana", 13.0f,
                           m_active ? juce::Font::bold : juce::Font::plain));
    g.drawFittedText (m_label, bounds, juce::Justification::centred, 1);
}

// =============================================================================
// ProgressionVisualizerComponent
// =============================================================================
ProgressionVisualizerComponent::ProgressionVisualizerComponent()
{
    setInterceptsMouseClicks (false, false);
}

void ProgressionVisualizerComponent::setChords (const std::vector<ResolvedChord>& chords)
{
    m_blocks.clear();
    for (auto& c : chords)
    {
        auto* b = m_blocks.add (new ChordBlockComponent());
        b->setLabel (juce::String (c.displayName));
        addAndMakeVisible (b);
    }
    resized();
}

void ProgressionVisualizerComponent::setActiveChord (int index)
{
    for (int i = 0; i < m_blocks.size(); ++i)
        m_blocks[i]->setActive (i == index);
}

void ProgressionVisualizerComponent::resized()
{
    if (m_blocks.isEmpty()) return;
    int w = getWidth() / m_blocks.size();
    for (int i = 0; i < m_blocks.size(); ++i)
        m_blocks[i]->setBounds (i * w, 0, w, getHeight());
}

void ProgressionVisualizerComponent::paint (juce::Graphics& g)
{
    g.fillAll (Pal::panel.darker (0.3f));
    if (m_blocks.isEmpty())
    {
        g.setColour (Pal::textDim);
        g.setFont (14.0f);
        g.drawText ("No progression loaded", getLocalBounds(), juce::Justification::centred);
    }
}

// =============================================================================
// PianoRollPreviewComponent
// =============================================================================
PianoRollPreviewComponent::PianoRollPreviewComponent()
{
    setInterceptsMouseClicks (false, false);
}

void PianoRollPreviewComponent::setNotes (const std::vector<int>& midiNotes, int highlightStep)
{
    m_notes        = midiNotes;
    m_highlightStep = highlightStep;
    repaint();
}

void PianoRollPreviewComponent::paint (juce::Graphics& g)
{
    g.fillAll (Pal::panel);

    if (m_notes.empty()) return;

    const int w = getWidth();
    const int h = getHeight();

    // Determine pitch range to display
    int minNote = 127, maxNote = 0;
    for (int n : m_notes) { minNote = std::min (minNote, n); maxNote = std::max (maxNote, n); }
    // Add a margin of 2 semitones above/below for readability
    minNote = std::max (0,   minNote - 2);
    maxNote = std::min (127, maxNote + 2);
    int range = maxNote - minNote + 1;
    if (range < 1) range = 1;

    float noteH = (float)h / (float)range;

    // Draw piano key background
    for (int pitch = minNote; pitch <= maxNote; ++pitch)
    {
        int pitchClass = pitch % 12;
        bool isBlack = (pitchClass == 1 || pitchClass == 3 || pitchClass == 6 ||
                        pitchClass == 8 || pitchClass == 10);
        int y = h - (int)((pitch - minNote + 1) * noteH);
        g.setColour (isBlack ? Pal::pianoBlack : Pal::pianoWhite);
        g.fillRect (0, y, w, (int)noteH);
        // Grid line
        g.setColour (Pal::bg.darker (0.5f));
        g.drawHorizontalLine (y, 0.0f, (float)w);
    }

    // Draw note blocks
    int numSteps = (int)m_notes.size();
    if (numSteps == 0) return;
    float stepW = (float)w / (float)numSteps;

    for (int step = 0; step < numSteps; ++step)
    {
        int pitch = m_notes[static_cast<size_t>(step)];
        int y = h - (int)((pitch - minNote + 1) * noteH);
        float x = step * stepW;

        bool active = (step == m_highlightStep);
        g.setColour (active ? Pal::noteOn : Pal::highlight.withAlpha (0.55f));
        g.fillRoundedRectangle (x + 1.0f, (float)y + 1.0f,
                                stepW - 2.0f, noteH - 2.0f, 2.0f);
    }

    // Label
    g.setColour (Pal::textDim);
    g.setFont (10.0f);
    g.drawText ("Arp Preview", getLocalBounds().removeFromTop (14),
                juce::Justification::topLeft);
}

// =============================================================================
// LabeledComboBox
// =============================================================================
LabeledComboBox::LabeledComboBox (const juce::String& labelText)
{
    m_label.setText (labelText, juce::dontSendNotification);
    m_label.setColour (juce::Label::textColourId, Pal::textDim);
    m_label.setFont (juce::Font (11.0f));
    m_label.setJustificationType (juce::Justification::centredLeft);

    m_combo.setColour (juce::ComboBox::backgroundColourId, Pal::accent);
    m_combo.setColour (juce::ComboBox::textColourId,       Pal::text);
    m_combo.setColour (juce::ComboBox::arrowColourId,      Pal::textDim);
    m_combo.setColour (juce::ComboBox::outlineColourId,    Pal::accent.brighter (0.2f));

    addAndMakeVisible (m_label);
    addAndMakeVisible (m_combo);
}

void LabeledComboBox::resized()
{
    auto b = getLocalBounds();
    m_label.setBounds (b.removeFromTop (16));
    m_combo.setBounds (b);
}

// =============================================================================
// LabeledSlider
// =============================================================================
LabeledSlider::LabeledSlider (const juce::String& labelText)
{
    m_label.setText (labelText, juce::dontSendNotification);
    m_label.setColour (juce::Label::textColourId, Pal::textDim);
    m_label.setFont (juce::Font (11.0f));
    m_label.setJustificationType (juce::Justification::centredLeft);

    m_slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    m_slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 16);
    m_slider.setColour (juce::Slider::rotarySliderFillColourId,  Pal::highlight);
    m_slider.setColour (juce::Slider::rotarySliderOutlineColourId, Pal::accent);
    m_slider.setColour (juce::Slider::thumbColourId,             Pal::highlight.brighter());
    m_slider.setColour (juce::Slider::textBoxTextColourId,       Pal::textDim);
    m_slider.setColour (juce::Slider::textBoxBackgroundColourId, Pal::panel);
    m_slider.setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);

    addAndMakeVisible (m_label);
    addAndMakeVisible (m_slider);
}

void LabeledSlider::resized()
{
    auto b = getLocalBounds();
    m_label.setBounds (b.removeFromTop (14));
    m_slider.setBounds (b);
}

// =============================================================================
// ChordArpEditor
// =============================================================================
ChordArpEditor::ChordArpEditor (ChordArpProcessor& p)
    : AudioProcessorEditor (&p), m_proc (p)
{
    setSize (820, 540);
    getLookAndFeel().setColour (juce::ResizableWindow::backgroundColourId, Pal::bg);

    // ── Genre ─────────────────────────────────────────────────────────────
    for (auto& name : ProgressionPresets::getGenreNames())
        m_genreBox.getComboBox().addItem (name, m_genreBox.getComboBox().getNumItems() + 1);
    m_genreBox.getComboBox().setSelectedId (1, juce::dontSendNotification);
    m_genreBox.getComboBox().onChange = [this] { onGenreChanged(); };

    // ── Progression ───────────────────────────────────────────────────────
    populateProgressionBox (0);
    m_progressionBox.getComboBox().onChange = [this] { onProgressionChanged(); };

    // ── Root key ──────────────────────────────────────────────────────────
    static const char* noteNames[] = {
        "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
    };
    for (int i = 0; i < 12; ++i)
        m_keyBox.getComboBox().addItem (noteNames[i], i + 1);
    m_keyBox.getComboBox().setSelectedId (1, juce::dontSendNotification);
    m_keyBox.getComboBox().onChange = [this] { onKeyChanged(); };

    // ── Mode ──────────────────────────────────────────────────────────────
    auto modeNames = ChordEngine::getModeNames();
    for (int i = 0; i < modeNames.size(); ++i)
        m_modeBox.getComboBox().addItem (modeNames[i], i + 1);
    m_modeBox.getComboBox().setSelectedId (1, juce::dontSendNotification);
    m_modeBox.getComboBox().onChange = [this] { onModeChanged(); };

    // ── Suggest button ────────────────────────────────────────────────────
    m_suggestBtn.setColour (juce::TextButton::buttonColourId,   Pal::highlight);
    m_suggestBtn.setColour (juce::TextButton::textColourOnId,   juce::Colours::white);
    m_suggestBtn.setColour (juce::TextButton::textColourOffId,  juce::Colours::white);
    m_suggestBtn.onClick = [this] { onSuggest(); };

    // ── Note Rate combobox ────────────────────────────────────────────────
    m_rateBox.getComboBox().addItem ("1/4",  1);
    m_rateBox.getComboBox().addItem ("1/8",  2);
    m_rateBox.getComboBox().addItem ("1/16", 3);
    m_rateBox.getComboBox().addItem ("1/32", 4);
    m_rateAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        m_proc.getAPVTS(), "rate", m_rateBox.getComboBox());

    // ── Direction combobox ────────────────────────────────────────────────
    m_directionBox.getComboBox().addItem ("Up",        1);
    m_directionBox.getComboBox().addItem ("Down",      2);
    m_directionBox.getComboBox().addItem ("Random",    3);
    m_directionBox.getComboBox().addItem ("Inside-Out",4);
    m_dirAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        m_proc.getAPVTS(), "direction", m_directionBox.getComboBox());

    // ── Octave range slider ───────────────────────────────────────────────
    m_octaveSlider.getSlider().setRange (1.0, 3.0, 1.0);
    m_octaveAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        m_proc.getAPVTS(), "octaveRange", m_octaveSlider.getSlider());

    // ── Gate slider ───────────────────────────────────────────────────────
    m_gateSlider.getSlider().setRange (0.05, 1.0, 0.01);
    m_gateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        m_proc.getAPVTS(), "gate", m_gateSlider.getSlider());

    // ── Velocity slider ───────────────────────────────────────────────────
    m_velSlider.getSlider().setRange (0.0, 1.0, 0.01);
    m_velAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        m_proc.getAPVTS(), "velocity", m_velSlider.getSlider());

    // ── Velocity randomise toggle ─────────────────────────────────────────
    m_velRandBtn.setColour (juce::ToggleButton::textColourId,      Pal::text);
    m_velRandBtn.setColour (juce::ToggleButton::tickColourId,       Pal::highlight);
    m_velRandBtn.setColour (juce::ToggleButton::tickDisabledColourId, Pal::textDim);
    m_velRandAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        m_proc.getAPVTS(), "velocityRand", m_velRandBtn);

    // ── Add all to editor ─────────────────────────────────────────────────
    addAndMakeVisible (m_genreBox);
    addAndMakeVisible (m_progressionBox);
    addAndMakeVisible (m_keyBox);
    addAndMakeVisible (m_modeBox);
    addAndMakeVisible (m_suggestBtn);
    addAndMakeVisible (m_progVisualizer);
    addAndMakeVisible (m_pianoRoll);
    addAndMakeVisible (m_rateBox);
    addAndMakeVisible (m_directionBox);
    addAndMakeVisible (m_octaveSlider);
    addAndMakeVisible (m_gateSlider);
    addAndMakeVisible (m_velSlider);
    addAndMakeVisible (m_velRandBtn);

    // Refresh UI to match initial state
    onGenreChanged();

    startTimerHz (24); // ~24 fps refresh for visualizer
}

ChordArpEditor::~ChordArpEditor()
{
    stopTimer();
}

// ─────────────────────────────────────────────────────────────────────────────
void ChordArpEditor::paint (juce::Graphics& g)
{
    g.fillAll (Pal::bg);

    // Section labels
    g.setColour (Pal::textDim);
    g.setFont (juce::Font (11.0f, juce::Font::italic));

    // Panel backgrounds
    auto b = getLocalBounds();
    int topH   = 80;
    int vizH   = 90;
    int rollH  = 100;
    int arpH   = 140;

    g.setColour (Pal::panel);
    g.fillRoundedRectangle (b.removeFromTop (topH).toFloat(), 0.0f);

    // Section dividers
    g.setColour (Pal::accent);
    g.drawHorizontalLine (topH,           0.0f, (float)getWidth());
    g.drawHorizontalLine (topH + vizH,    0.0f, (float)getWidth());
    g.drawHorizontalLine (topH + vizH + rollH, 0.0f, (float)getWidth());

    // "Arpeggiator" label
    g.setColour (Pal::textDim);
    g.setFont (juce::Font (11.0f));
    g.drawText ("ARPEGGIATOR", juce::Rectangle<int> (10, topH + vizH + rollH + 4, 120, 14),
                juce::Justification::centredLeft);
}

// ─────────────────────────────────────────────────────────────────────────────
void ChordArpEditor::resized()
{
    const int W = getWidth();
    const int pad = 8;

    // ── Top row (genre, progression, key, mode, suggest) ──────────────────
    auto topRow = juce::Rectangle<int> (pad, 8, W - pad * 2, 64);
    int comboW  = (W - pad * 2 - 90 - pad * 4) / 4;

    m_genreBox.setBounds       (topRow.removeFromLeft (comboW));  topRow.removeFromLeft (pad);
    m_progressionBox.setBounds (topRow.removeFromLeft (comboW));  topRow.removeFromLeft (pad);
    m_keyBox.setBounds         (topRow.removeFromLeft (comboW));  topRow.removeFromLeft (pad);
    m_modeBox.setBounds        (topRow.removeFromLeft (comboW));  topRow.removeFromLeft (pad);
    m_suggestBtn.setBounds     (topRow.withHeight (30).withY (topRow.getY() + 16));

    // ── Progression visualizer ─────────────────────────────────────────────
    m_progVisualizer.setBounds (pad, 80, W - pad * 2, 90);

    // ── Piano roll preview ─────────────────────────────────────────────────
    m_pianoRoll.setBounds (pad, 174, W - pad * 2, 100);

    // ── Arp controls ──────────────────────────────────────────────────────
    int arpY = 282;
    int arpH = getHeight() - arpY - pad;
    int arpW = W - pad * 2;

    // Two combos + four knobs + toggle button
    // Lay them left to right with equal spacing
    int numControls = 7;
    int ctrlW = arpW / numControls;

    m_rateBox.setBounds       (pad + 0 * ctrlW, arpY + 20, ctrlW - pad, arpH - 20);
    m_directionBox.setBounds  (pad + 1 * ctrlW, arpY + 20, ctrlW - pad, arpH - 20);
    m_octaveSlider.setBounds  (pad + 2 * ctrlW, arpY + 14, ctrlW - pad, arpH - 14);
    m_gateSlider.setBounds    (pad + 3 * ctrlW, arpY + 14, ctrlW - pad, arpH - 14);
    m_velSlider.setBounds     (pad + 4 * ctrlW, arpY + 14, ctrlW - pad, arpH - 14);
    m_velRandBtn.setBounds    (pad + 5 * ctrlW, arpY + arpH / 2, ctrlW * 2 - pad, 24);
}

// ─────────────────────────────────────────────────────────────────────────────
// Timer: refresh visualizer to show active chord + piano roll step
// ─────────────────────────────────────────────────────────────────────────────
void ChordArpEditor::timerCallback()
{
    m_progVisualizer.setActiveChord (m_proc.getCurrentChordIndex());
    m_pianoRoll.setNotes (m_proc.getArpPreviewSequence(),
                          m_proc.getCurrentStepIndex());
}

// ─────────────────────────────────────────────────────────────────────────────
// UI callbacks
// ─────────────────────────────────────────────────────────────────────────────
void ChordArpEditor::populateProgressionBox (int genreIndex)
{
    auto& combo = m_progressionBox.getComboBox();
    combo.clear (juce::dontSendNotification);
    const auto& progs = ProgressionPresets::getProgressions (genreIndex);
    for (int i = 0; i < (int)progs.size(); ++i)
        combo.addItem (juce::String (progs[static_cast<size_t>(i)].name), i + 1);
    if (!progs.empty())
        combo.setSelectedId (1, juce::dontSendNotification);
}

void ChordArpEditor::onGenreChanged()
{
    m_currentGenreIndex = m_genreBox.getComboBox().getSelectedItemIndex();
    populateProgressionBox (m_currentGenreIndex);
    m_currentProgressionIdx = 0;
    m_proc.loadProgression (m_currentGenreIndex, m_currentProgressionIdx);
    m_progVisualizer.setChords (m_proc.getCurrentProgression());
}

void ChordArpEditor::onProgressionChanged()
{
    m_currentProgressionIdx = m_progressionBox.getComboBox().getSelectedItemIndex();
    m_proc.loadProgression (m_currentGenreIndex, m_currentProgressionIdx);
    m_progVisualizer.setChords (m_proc.getCurrentProgression());
}

void ChordArpEditor::onKeyChanged()
{
    int root = m_keyBox.getComboBox().getSelectedItemIndex(); // 0=C … 11=B
    m_proc.setKey (root);
    m_progVisualizer.setChords (m_proc.getCurrentProgression());
}

void ChordArpEditor::onModeChanged()
{
    int modeIdx = m_modeBox.getComboBox().getSelectedItemIndex();
    m_proc.setMode (static_cast<ScaleMode> (modeIdx));
    m_progVisualizer.setChords (m_proc.getCurrentProgression());
}

void ChordArpEditor::onSuggest()
{
    m_proc.suggestProgression (m_currentGenreIndex);
    m_progVisualizer.setChords (m_proc.getCurrentProgression());
    // Sync the progression combo to reflect the new selection
    // (suggestion cycles through all progressions in the genre)
}
