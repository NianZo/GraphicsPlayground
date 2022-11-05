cmake_minimum_required(VERSION 3.16)

set (Recipe_Name "GraphicsPlayground")

# Specify project name
project(${Recipe_Name})

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    message("Building in Debug")
    add_compile_options(--coverage -O0 -fprofile-instr-generate -fcoverage-mapping)
endif()

# Standard compiler warnings
add_library(project_warnings INTERFACE)
include(CompilerWarnings.cmake)
set_project_warnings(project_warnings)

add_subdirectory(newRenderer)
add_subdirectory(ui)

option(ENABLE_TESTING "Enable Test Builds" ON)

if(ENABLE_TESTING)
    message("Building tests")
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()





