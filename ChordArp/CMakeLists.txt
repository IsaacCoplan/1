cmake_minimum_required(VERSION 3.22)
project(ChordArp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Fetch JUCE ─────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ── Plugin target ──────────────────────────────────────────────────────────
juce_add_plugin(ChordArp
    COMPANY_NAME            "ChordArp"
    PLUGIN_MANUFACTURER_CODE CArp
    PLUGIN_CODE             CAr1

    FORMATS                 VST3 Standalone
    PRODUCT_NAME            "ChordArp"

    IS_MIDI_EFFECT          FALSE
    NEEDS_MIDI_INPUT        FALSE
    NEEDS_MIDI_OUTPUT       TRUE
    IS_SYNTH                TRUE          # Required for MIDI-out-only in FL Studio

    PLUGIN_VST3_CATEGORY    Instrument

    VST3_CATEGORIES         "Instrument"
    AU_MAIN_TYPE            kAudioUnitType_MusicDevice
)

# ── Sources ────────────────────────────────────────────────────────────────
target_sources(ChordArp PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/ChordEngine.cpp
    Source/ArpEngine.cpp
    Source/ProgressionPresets.cpp
)

# ── Include dirs ──────────────────────────────────────────────────────────
target_include_directories(ChordArp PRIVATE Source)

# ── JUCE modules ──────────────────────────────────────────────────────────
target_link_libraries(ChordArp
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ── Compile definitions ────────────────────────────────────────────────────
target_compile_definitions(ChordArp
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
