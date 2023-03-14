find_package(Vulkan REQUIRED COMPONENTS glslangValidator)
find_program(glslangValidator_executable NAMES glslangValidator HINTS Vulkan::glslangValidator)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB SHADERS
	${CMAKE_CURRENT_SOURCE_DIR}/*.vert
	${CMAKE_CURRENT_SOURCE_DIR}/*.frag
)
message("glslandValidator: ${glslangValidator_executable}")
foreach(source IN LISTS SHADERS)
	get_filename_component(FILENAME ${source} NAME_WE)
	get_filename_component(EXTENSION ${source} EXT)
	string(REPLACE "." "_" EXTENSION ${EXTENSION})
	add_custom_command(
		COMMAND
			${glslangValidator_executable}
			-V100
			-o ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}${EXTENSION}.spv
			${source}
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}${EXTENSION}.spv
		DEPENDS ${source}
		COMMENT "Compiling ${FILENAME}"
	)
	list(APPEND SPV_SHADERS ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}${EXTENSION}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})