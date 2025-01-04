cmake_minimum_required(VERSION 3.22)

project(AUDIO_PLUGIN_EXAMPLE VERSION 0.0.1)

add_subdirectory(JUCE)

juce_add_plugin(AudioPluginExample
    VERSION 1.0.0
    COMPANY_NAME "CoconutAudio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Coau
    PLUGIN_CODE Twnb
    FORMATS VST3
    PRODUCT_NAME "There will not be blood")

juce_generate_juce_header(AudioPluginExample)

target_sources(AudioPluginExample
    PRIVATE
        "src/PluginEditor.cpp"
        "src/PluginProcessor.cpp"
        "src/LevelMeter.cpp"
        "src/SpectrumAnalyzer.cpp")

target_compile_definitions(AudioPluginExample
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

juce_add_binary_data(AudioPluginData 
    SOURCES 
        "res/coconut_audio.png"
        "res/kraut____typefuck11.ttf"
        "res/rust_texture.png")

target_link_libraries(AudioPluginExample
    PRIVATE
        AudioPluginData
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
