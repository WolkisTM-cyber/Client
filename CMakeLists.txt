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
    src/modules/packet/PacketUtil.cpp
    src/config/ConfigManager.cpp
    src/render/Renderer.cpp
    src/command/CommandManager.cpp
    src/gui/ClickGUI.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${JNI_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    gdi32
    user32
    comctl32
    advapi32
    opengl32
    glu32
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

# Injector subproject
add_subdirectory(injector)
