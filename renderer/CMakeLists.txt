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
endif()

set(RENDERER_LIB "VulkanRenderer")

# Gather list of header and source files for compilation
file(GLOB_RECURSE VK_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
file(GLOB_RECURSE VK_HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.*)

add_library(${RENDERER_LIB} STATIC ${VK_CPP_FILES} ${VK_HPP_FILES})
set_property(TARGET ${RENDERER_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(${RENDERER_LIB} PUBLIC ${CMAKE_SOURCE_DIR}/external/stb)
target_link_libraries(${RENDERER_LIB} PUBLIC ${Vulkan_LIBRARIES})
target_include_directories(${RENDERER_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${RENDERER_LIB} PUBLIC ${VULKAN_PATH}/include)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_link_libraries(${RENDERER_LIB} PRIVATE --coverage)
endif()









