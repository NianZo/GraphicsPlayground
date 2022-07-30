set(MAIN_BINARY "GraphicsPlayground")

# Define include path
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()
set(CMAKE_AUTOUIC ON)

# Gather list of header and source files for compilation
file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
set(AUTOGEN_BUILD_DIR ${CMAKE_SOURCE_DIR}/build/default)
qt6_wrap_ui(CPP_FILES MainGUI.ui)

add_executable(${MAIN_BINARY} ${CPP_FILES} ${HPP_FILES})
target_include_directories(${MAIN_BINARY} PUBLIC ${CMAKE_SOURCE_DIR}/renderer/include)
target_include_directories(${MAIN_BINARY} PUBLIC ${AUTOGEN_BUILD_DIR})
target_include_directories(${MAIN_BINARY} PUBLIC ${QT_INCLUDES})

target_link_libraries(${MAIN_BINARY} PRIVATE Qt6::Core Qt6::Widgets)
target_link_libraries(${MAIN_BINARY} PUBLIC "VulkanRenderer")