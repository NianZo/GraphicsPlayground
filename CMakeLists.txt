cmake_minimum_required(VERSION 3.16)

set (Recipe_Name "GraphicsPlayground")

# Specify project name
project(${Recipe_Name})

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE Debug)
message("build type: ${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE MATCHES Debug)
    message("Building in Debug")
    add_compile_options(-O0 -g)
    #add_compile_options(-O0 --coverage -fprofile-instr-generate -fcoverage-mapping)
    add_compile_options(-O0 --coverage -fprofile-generate)
    add_compile_options(-O0 -fprofile-arcs -ftest-coverage)
    #add_compile_options(-O0 -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

message("Compiler: ${CMAKE_CXX_COMPILER}")

# Standard compiler warnings
add_library(project_warnings INTERFACE)
include(CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# Set up clang-tidy
#find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
#set(CLANG_TIDY_COMMAND 

add_subdirectory(renderer)
add_subdirectory(ui)
add_subdirectory(shaders)

option(ENABLE_TESTING "Enable Test Builds" ON)

if(ENABLE_TESTING)
    message("Building tests")
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()





