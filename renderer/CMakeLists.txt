set(RENDERER_LIB "VulkanRenderer")

# Gather list of header and source files for compilation
file(GLOB_RECURSE VK_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
file(GLOB_RECURSE VK_HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.*)

add_library(${RENDERER_LIB} STATIC ${VK_CPP_FILES} ${VK_HPP_FILES})
#target_include_directories(${RENDERER_LIB} PUBLIC ${QT_INCLUDES})
#target_link_libraries(${RENDERER_LIB} PRIVATE Qt6::Core Qt6::Widgets)
target_link_libraries(${RENDERER_LIB} PUBLIC ${Vulkan_LIBRARIES})
target_include_directories(${RENDERER_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${RENDERER_LIB} PUBLIC ${VULKAN_PATH}/include)