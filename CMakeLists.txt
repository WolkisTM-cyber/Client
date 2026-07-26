cmake_minimum_required(VERSION 3.10)
project(Client VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(JNI REQUIRED)

set(SOURCES
    src/dllmain.cpp
    src/GUI.cpp
    src/modules/Module.cpp
    src/modules/ModuleManager.cpp
    src/modules/JNIHelper.cpp
)

set(HEADERS
    src/GUI.h
    src/modules/Module.h
    src/modules/Category.h
    src/modules/Setting.h
    src/modules/ModuleManager.h
    src/modules/JNIHelper.h
    src/modules/combat/Velocity.h
    src/modules/combat/MurdererFinder.h
    src/modules/combat/KillAura.h
    src/modules/combat/Reach.h
    src/modules/combat/Criticals.h
    src/modules/combat/AutoBlock.h
    src/modules/movement/AutoSprint.h
    src/modules/movement/KeepSprint.h
    src/modules/movement/Speed.h
    src/modules/movement/Flight.h
    src/modules/movement/NoSlowdown.h
    src/modules/movement/Step.h
    src/modules/movement/Glide.h
    src/modules/movement/LongJump.h
    src/modules/movement/NoWeb.h
    src/modules/movement/Strafe.h
    src/modules/movement/IceSpeed.h
    src/modules/visual/FullBright.h
    src/modules/visual/NoHurtCam.h
    src/modules/visual/NoRotate.h
    src/modules/visual/NoBob.h
    src/modules/visual/Tracers.h
    src/modules/visual/ESP.h
    src/modules/visual/ChestESP.h
    src/modules/visual/Chams.h
    src/modules/player/NoFall.h
    src/modules/player/AntiVoid.h
    src/modules/player/Sneak.h
    src/modules/player/Scaffold.h
    src/modules/player/FastPlace.h
    src/modules/player/AutoTool.h
    src/modules/misc/Timer.h
    src/modules/misc/AntiBot.h
    src/modules/misc/NoClickDelay.h
    src/modules/misc/Spammer.h
    src/modules/misc/Derp.h
    src/modules/misc/AutoL.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${JNI_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    gdi32
    user32
    comctl32
    advapi32
)

if(MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${PROJECT_NAME} PRIVATE /O2 /MT)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE /MTd)
    endif()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif()
