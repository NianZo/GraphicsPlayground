cmake_minimum_required(VERSION 3.16)

set (Recipe_Name "GraphicsPlayground")

# Specify project name
project(${Recipe_Name})

#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(renderer)
add_subdirectory(ui)

add_subdirectory(test)





