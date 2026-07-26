cmake_minimum_required(VERSION 3.10)
project(Injector VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} WIN32 injector.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE advapi32)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /O2 /MT)
endif()
