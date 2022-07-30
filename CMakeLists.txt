cmake_minimum_required(VERSION 3.16)

set (Recipe_Name "GraphicsPlayground")

# Specify project name
project(${Recipe_Name})

add_subdirectory(renderer)
add_subdirectory(ui)

add_subdirectory(test)





