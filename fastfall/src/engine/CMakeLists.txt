cmake_minimum_required (VERSION 3.25)

target_sources(fastfall PRIVATE
    EngineRunnable.cpp
    audio.cpp
    Engine.cpp
    state/EngineStateHandler.cpp
    state/EngineState.cpp
    imgui/ImGuiFrame.cpp
    imgui/ImGuiContent.cpp
    time/FixedEngineClock.cpp
    input/InputHandle.cpp
    input/GamepadInput.cpp
    input/InputSourceRealtime.cpp
    input/InputConfig.cpp
    input/InputSourceRecord.cpp
    input/Gamepad.hpp
    input/Mouse.cpp
    input/Input_Def.cpp
    input/InputSource.cpp
    input/InputRecord.cpp
    input/InputState.cpp
)