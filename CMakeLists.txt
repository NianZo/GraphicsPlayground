cmake_minimum_required(VERSION 3.16)

set (Recipe_Name "GraphicsPlayground")

# Specify project name
project(${Recipe_Name})

# AUTO_LOCATE_VULKAN - accepted value ON or OFF
# ON - Use CMake to auto locate the Vulkan SDK.
# OFF - Vulkan SDK path can be specified manually
# This is helpful to test the build on various Vulkan versions.
option(AUTO_LOCATE_VULKAN "AUTO_LOCATE_VULKAN" ON)

if (AUTO_LOCATE_VULKAN)
    message(STATUS "Attempting to auto locate Vulkan using CMake...")

    # Find Vulkan Path using CMake's Vulkan Module
    # This will return boolean 'Vulkan_FOUND' indicating
    # the status of finding as success (TRUE) or fail (FALSE).
    # Include directory path - 'Vulkan_INCLUDE_DIRS'
    # and 'Vulkan_LIBRARY' with required libraries.
    find_package(Vulkan)

    # Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
    if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
        set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
        STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
    endif()

    if (NOT Vulkan_FOUND)
        # CMake may fail to locate the libraries but could be able to
        # provide some path in VulkanSDK include directory variable
        # 'Vulkan_INCLUDE_DIRS', try to extract path from this
        message(STATUS "Failed to locate VulkanSDK, retrying from 'Vulkan_INCLUDE_DIRS' variable")

        # Check if Vulkan path is valid, if not switch to manual mode
        if (EXISTS "${VULKAN_PATH}")
            message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
        else()
            message(ERROR " Unable to locate VulkanSDK. Please turn off auto locate option by specifying 'AUTO_LOCATE_VULKAN' as 'OFF'")
            message(ERROR " and specify manual path using 'VULKAN_SDK' and 'VULKAN_VERSION' variables in CMakeLists.txt")
            return()
        endif()
    endif()
else()
    message(STATUS "Attempting to locate VulkanSDK using manual path...")
    set(VULKAN_SDK "/home/nic/Staging/VulkanSDK/1.3.211.0")
    set(VULKAN_VERSION "1.3.211.0")
    set(VULKAN_PATH "${VULKAN_SDK}")
    message(STATUS "Using manual specified path: ${VULKAN_PATH}")

    # Check if manual set path exists
    if (NOT EXISTS "${VULKAN_PATH}")
        message(ERROR "Unable to locate this VulkanSDK path: ${VULKAN_PATH}")
        return()
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

include_directories("${CMAKE_SOURCE_DIR}/external/stb")

find_package(glfw3 REQUIRED)
link_libraries(glfw)

include_directories(AFTER ${VULKAN_PATH}/include)

link_directories(${VULKAN_PATH}/bin)
link_libraries(${Vulkan_LIBRARIES})

# Define include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Gather list of header and source files for compilation
file(GLOB_RECURSE VK_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
file(GLOB_RECURSE VK_HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.*)



# Gather list of header and source files for compilation
file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/hellovulkanwidget/*.cpp)
file(GLOB_RECURSE HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/hellovulkanwidget/*.h)

#set(CPP_FILES "${CPP_FILES} ./shared/trianglerenderer.cpp")
#set(HPP_FILES "${HPP_FILES} ./shared/trianglerenderer.h")
include_directories(${QT_INCLUDES})
# Build project, provide name and include files to be compiled
add_executable(${Recipe_Name} ${CPP_FILES} ${HPP_FILES} shared/trianglerenderer.cpp shared/trianglerenderer.h ${VK_CPP_FILES} ${VK_HPP_FILES})

target_link_libraries(${Recipe_Name} PRIVATE Qt6::Core Qt6::Widgets)
target_link_libraries(${Recipe_Name} ${VULKAN_LIB_LIST})




