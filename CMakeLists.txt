cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(jsnmk LANGUAGES C)

add_library(jsnmk INTERFACE)
target_sources(jsnmk INTERFACE
    src/jsnmk.c
    src/jsnmk.h
)
target_include_directories(jsnmk INTERFACE src)

option(ENABLE_TESTS "Enable test compilation" ${PROJECT_IS_TOP_LEVEL})
option(ENABLE_SANITIZERS "Build test executables with AddressSanitizer/UBSan" OFF)

if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

