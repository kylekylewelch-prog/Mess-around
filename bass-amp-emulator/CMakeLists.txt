cmake_minimum_required(VERSION 3.22)

project(BassAmpStudio VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BASSAMP_BUILD_TESTS  "Build the DSP test suite"     ON)
option(BASSAMP_BUILD_PLUGIN "Build the app and VST3 plugin" ON)

# ASIO SDK: bundled under third_party/asiosdk (GPLv3-licensed copy as of SDK 2.3.4).
# A path override is still supported for custom installs.
set(_BUNDLED_ASIO "${CMAKE_CURRENT_SOURCE_DIR}/third_party/asiosdk")
if(EXISTS "${_BUNDLED_ASIO}/common/asio.h")
    set(_DEFAULT_ASIO_PATH "${_BUNDLED_ASIO}")
else()
    set(_DEFAULT_ASIO_PATH "")
endif()
set(BASSAMP_ASIO_SDK_PATH "${_DEFAULT_ASIO_PATH}" CACHE PATH
    "Path to the Steinberg ASIO SDK root (the folder containing common/ and host/). \
Defaults to the bundled copy in third_party/asiosdk.")

set(BASSAMP_JUCE_TAG "8.0.15" CACHE STRING "JUCE release tag to build against")

# ---------------------------------------------------------------------------
#  DSP core: no framework dependency, so it can be built and tested anywhere.
# ---------------------------------------------------------------------------
file(GLOB BASSAMP_CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/src/*.cpp")

add_library(bassamp_core STATIC ${BASSAMP_CORE_SOURCES})
target_include_directories(bassamp_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/include")
set_target_properties(bassamp_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    target_compile_options(bassamp_core PRIVATE /W4 /fp:fast)
else()
    target_compile_options(bassamp_core PRIVATE -Wall -Wextra -ffast-math)
endif()

# ---------------------------------------------------------------------------
#  Tests
# ---------------------------------------------------------------------------
if(BASSAMP_BUILD_TESTS)
    enable_testing()
    file(GLOB BASSAMP_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
    add_executable(bassamp_tests ${BASSAMP_TEST_SOURCES})
    target_link_libraries(bassamp_tests PRIVATE bassamp_core)
    target_include_directories(bassamp_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_test(NAME dsp COMMAND bassamp_tests)
endif()

# ---------------------------------------------------------------------------
#  Application (standalone + VST3)
# ---------------------------------------------------------------------------
if(BASSAMP_BUILD_PLUGIN)
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        ${BASSAMP_JUCE_TAG}
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)

    juce_add_plugin(BassAmpStudio
        COMPANY_NAME            "BassAmp"
        PRODUCT_NAME            "BassAmp Studio"
        PLUGIN_MANUFACTURER_CODE Bamp
        PLUGIN_CODE             Bas1
        FORMATS                 Standalone VST3
        IS_SYNTH                FALSE
        NEEDS_MIDI_INPUT        TRUE
        NEEDS_MIDI_OUTPUT       FALSE
        IS_MIDI_EFFECT          FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        COPY_PLUGIN_AFTER_BUILD FALSE
        VST3_CATEGORIES         Fx Distortion
        DESCRIPTION             "Bass amp, cabinet and pedalboard emulator")

    target_sources(BassAmpStudio PRIVATE
        app/Source/PluginProcessor.cpp
        app/Source/PluginEditor.cpp
        app/Source/ParameterBridge.cpp
        app/Source/TrackPlayer.cpp
        app/Source/Gui/LookAndFeel.cpp
        app/Source/Gui/Panels.cpp
        app/Source/Gui/UtilityPanels.cpp)

    target_include_directories(BassAmpStudio PRIVATE app/Source)

    target_compile_definitions(BassAmpStudio PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_USE_MP3AUDIOFORMAT=1      # so backing tracks can be MP3s
        JUCE_USE_FLAC=1
        JUCE_USE_OGGVORBIS=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        # The standalone opens its own device on launch; without this it starts
        # muted and the first thing a player does is think it is broken.
        JUCE_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE=0)

    if(WIN32 AND BASSAMP_ASIO_SDK_PATH)
        message(STATUS "Building with ASIO support from ${BASSAMP_ASIO_SDK_PATH}")
        target_compile_definitions(BassAmpStudio PUBLIC JUCE_ASIO=1)
        target_include_directories(BassAmpStudio PRIVATE "${BASSAMP_ASIO_SDK_PATH}/common")
    elseif(WIN32)
        message(STATUS "No ASIO SDK path given - the app will use WASAPI/DirectSound only. "
                       "See docs/BUILDING_WINDOWS.md to enable ASIO.")
    endif()

    target_link_libraries(BassAmpStudio PRIVATE
        bassamp_core
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_extra
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
endif()
