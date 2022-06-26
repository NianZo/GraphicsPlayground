cmake_minimum_required(VERSION 3.15)

set (Recipe_Name "GraphicsPlayground")

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

# Specify project name
project(${Recipe_Name})

set(CMAKE_BUILD_PARALLEL_LEVEL 24)
# GLI SETUP
#set (EXTDIR "${CMAKE_SOURCE_DIR}/../../external/gli")
#set (GLIINCLUDES "${EXTDIR}")
#get_filename_component(GLIINC_PREFIX "${GLIINCLUDES}" ABSOLUTE)
#if(NOT EXISTS ${GLIINC_PREFIX})
#	message(FATAL_ERROR "Necessary gli headers do not exist: " ${GLIINC_PREFIX})
#endif()
include_directories(${GLIINC_PREFIX})
include_directories("${CMAKE_SOURCE_DIR}/external/stb")

# Add preprocessor definitions here
#add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
find_package(glfw3 REQUIRED)
link_libraries(glfw)
#include_directories(AFTER /usr/include/GLFW)
# Add 'vulkan-1' library for built Vulkan application
#set(VULKAN_LINK_LIST "libvulkan")

#if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # Include Vulkan header files from VulkanSDK
    include_directories(AFTER ${VULKAN_PATH}/include)

    # Link directory for vulkan-1
    link_directories(${VULKAN_PATH}/bin)
    link_libraries(${Vulkan_LIBRARIES})
#endif()

# Bunch the header and source files together
if (WIN32)
    source_group("include" REGULAR_EXPRESSION "include/*")
    source_group("source" REGULAR_EXPRESSION "source/*")
endif(WIN32)

# Define include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Gather list of header and source files for compilation
file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
file(GLOB_RECURSE HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.*)

find_package(wxWidgets REQUIRED COMPONENTS net core base)
include(${wxWidgets_USE_FILE})
message(STATUS ${wxWidgets_USE_FILE})
#add_executable(myapp main.cpp)

find_package(X11)



# Build project, provide name and include files to be compiled
add_executable(${Recipe_Name} ${CPP_FILES} ${HPP_FILES})

# Link the debug and release libraries to the project
target_link_libraries(${Recipe_Name} ${VULKAN_LIB_LIST})
target_link_libraries(${Recipe_Name} ${wxWidgets_LIBRARIES})
target_link_libraries(${Recipe_Name} ${X11_LIBRARIES})

# Define project properties
set_property(TARGET ${Recipe_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY                ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Recipe_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Recipe_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Recipe_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Recipe_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/binaries)

# Define C++ version to be used for building the project
set_property(TARGET ${Recipe_Name} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${Recipe_Name} PROPERTY CXX_STANDARD_REQUIRED ON)

# Define C version to be used for building the project
set_property(TARGET ${Recipe_Name} PROPERTY C_STANDARD 99)
set_property(TARGET ${Recipe_Name} PROPERTY C_STANDARD_REQUIRED ON)


# BUILD_SPV_ON_COMPILE_TIME - accepted value ON or OFF
# ON - Reads the GLSL shader file and auto convert into SPIR-V form (.spv)
# This requires additional libraries support from VulkanSDK liike SPIRV glslang OGLCompiler OSDependent HLSL
# OFF - Only reads .spv files, which need to be compiled offline using glslangValidator.exe.
# For example: glslangValidator.exe <GLSL file name> -V -o <output filename in SPIR-V(.spv) form>
option (BUILD_SPV_ON_COMPILE_TIME "BUILD_SPV_ON_COMPILE_TIME" OFF)
if (BUILD_SPV_ON_COMPILE_TIME)

	# Preprocessor flag allows the solution to use glslang library functions
	add_definitions(-DAUTO_COMPILE_GLSL_TO_SPV)
	
	# GLSL - use Vulkan SDK's glslang library for compiling GLSL to SPV
	# This does not require offline conversion of GLSL shader to SPIR-V(.spv) form
	set(GLSLANGDIR "${VULKAN_PATH}/glslang")
	
	get_filename_component(GLSLANG_PREFIX "${GLSLANGDIR}" ABSOLUTE)
	
	# Check if glslang directory exists
	if (NOT EXISTS ${GLSLANG_PREFIX})
		message(FATAL_ERROR "Necessary glslang components do not exist: " ${GLSLANG_PREFIX})
	endif()
	
	# Include the glslang directory
	include_directories(${GLSLANG_PREFIX})
	
	# If compiling GLSL to SPV using we need the following libraries
	set(GLSLANG_LIBS SPIRV glslang OGLCompiler OSDependent HLSL)
	
	# Generate the list of files to link, per flavor
	foreach(x ${GLSLANG_LIBS})
		list(APPEND VULKAN_LIB_LIST debug ${x}d optimized ${x})
	endforeach()
	
	# Note: While configuring CMake for glslang we created the binaries in a "build" folder inside ${VULKAN_PATH}/glslang.
	# Therefore, you must edit the below lines for your custom path like <Your binary path>/OGLCompilersDLL, <Your binary path>/OSDependent/Windows
	set(VULKAN_GLSL_PATH /home/nic/Staging/VulkanSDK/1.3.204.1/source)
	link_directories(${VULKAN_PATH}/glslang/build/OGLCompilersDLL)
	link_directories(${VULKAN_PATH}/glslang/build/glslang/OSDependent/Windows)
	link_directories(${VULKAN_PATH}/glslang/build/glslang)
	link_directories(${VULKAN_PATH}/glslang/build/SPIRV)
	link_directories(${VULKAN_PATH}/glslang/build/hlsl)
	
endif()










