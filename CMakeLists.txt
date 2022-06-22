cmake_minimum_required (VERSION 2.6)

project (GraphicsPlayground)

find_package(wxWidgets REQUIRED COMPONENTS net core base)
include(${wxWidgets_USE_FILE})
message(STATUS ${wxWidgets_USE_FILE})
add_executable(myapp main.cpp)
target_link_libraries(myapp ${wxWidgets_LIBRARIES})