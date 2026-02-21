# My Synth Plugin

A minimal JUCE-based VST3 synthesizer plugin built with CMake.

## Features

- 8-voice polyphonic sine-wave synthesizer
- MIDI note input with velocity sensitivity
- Amplitude envelope with natural tail-off
- Master gain knob (APVTS-managed, state saved/restored)
- Standalone app target for quick testing without a DAW

## Project Structure

```
.
├── CMakeLists.txt          # Build system (downloads JUCE via FetchContent)
└── Source/
    ├── PluginProcessor.h   # Synth voices, sounds, AudioProcessor declaration
    ├── PluginProcessor.cpp # Audio/MIDI processing, state management
    ├── PluginEditor.h      # UI declaration
    └── PluginEditor.cpp    # UI implementation (gain knob)
```

## Requirements

| Tool | Minimum version |
|------|----------------|
| CMake | 3.22 |
| C++ compiler | C++17 (MSVC 2019+, GCC 9+, Clang 10+) |
| Git | any recent version |

**Linux only:** install the JUCE system dependencies first:

```bash
sudo apt-get install libasound2-dev libx11-dev libxinerama-dev \
    libxext-dev libfreetype6-dev libwebkit2gtk-4.0-dev libglu1-mesa-dev
```

## Building

```bash
# 1. Configure (downloads JUCE automatically)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Build VST3 + Standalone
cmake --build build --config Release

# 3. Installed artefacts (Linux/macOS)
#    VST3:       build/MySynthPlugin_artefacts/Release/VST3/
#    Standalone: build/MySynthPlugin_artefacts/Release/Standalone/
```

## Extending the Plugin

### Adding a new parameter

1. Add it to `createParameterLayout()` in `PluginProcessor.cpp`
2. Read it with `apvts.getRawParameterValue("id")` in `processBlock`
3. Wire a `juce::Slider` + `SliderAttachment` in the editor

### Replacing the sine oscillator

Swap `SineWaveVoice::renderNextBlock` with your own DSP – use
`juce::dsp::Oscillator`, a wavetable lookup, or any other algorithm.

### Adding more voices

Change `numVoices` in `MySynthPluginAudioProcessor` constructor.
