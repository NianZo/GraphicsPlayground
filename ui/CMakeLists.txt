set(MAIN_BINARY "GraphicsPlayground")

#Define include path
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt6_standard_project_setup()
set(CMAKE_AUTOUIC ON)

#Put a 'null'.clang - tidy file where generated Qt files will be so that Clang - Tidy ignores them
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/.clang-tidy" "---
Checks: '-*,llvm-twine-local'")

#Gather list of header and source files for compilation
file(GLOB_RECURSE CPP_FILES MainWindow.cpp)
file(GLOB_RECURSE HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
set(AUTOGEN_BUILD_DIR ${CMAKE_SOURCE_DIR}/build/default)
qt6_wrap_ui(CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/MainGUI.ui)

set(UI_LIB "MainUI")
add_library(${UI_LIB} STATIC ${CPP_FILES} ${HPP_FILES})


#set_property(TARGET ${MAIN_BINARY} PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET ${UI_LIB} PROPERTY CXX_STANDARD 20)
target_include_directories(${UI_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/newRenderer ${CMAKE_CURRENT_BINARY_DIR} ${Qt6Widgets_INCLUDE_DIRS})
#target_include_directories(${MAIN_BINARY} PUBLIC ${AUTOGEN_BUILD_DIR})
message(WARNING "QT_INCLUDES: ${Qt6Widgets_INCLUDE_DIRS}")


target_link_libraries(${UI_LIB} PRIVATE Qt6::Core Qt6::Widgets)
target_link_libraries(${UI_LIB} PUBLIC "VulkanRenderer")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    #    target_link_libraries(${UI_LIB} PRIVATE --coverage -fprofile-instr-generate -fcoverage-mapping)
endif ()

add_executable(${MAIN_BINARY} main.cpp)#${CPP_FILES} ${HPP_FILES})
set_property(TARGET ${MAIN_BINARY} PROPERTY CXX_STANDARD 20)
target_link_libraries(${MAIN_BINARY} PUBLIC ${UI_LIB})
target_link_libraries(${MAIN_BINARY} PRIVATE Qt6::Core Qt6::Widgets project_warnings)

target_include_directories(${MAIN_BINARY} PUBLIC ${Qt6Widgets_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/newRenderer)
