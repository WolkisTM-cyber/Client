cmake_minimum_required(VERSION 3.10)
project(MinecraftAutoSprint VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(JNI REQUIRED)

set(SOURCES
    src/dllmain.cpp
    src/AutoSprint.cpp
    src/GUI.cpp
    src/JVMHelper.cpp
)

set(HEADERS
    src/AutoSprint.h
    src/GUI.h
    src/JVMHelper.h
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
