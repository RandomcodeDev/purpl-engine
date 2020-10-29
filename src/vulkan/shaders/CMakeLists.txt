cmake_minimum_required(VERSION 3.10)

# Much of the logic here borrows from https://gist.github.com/evilactually/a0d191701cb48f157b05be7f74d79396

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64" OR ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
		set(GLSL_COMPILER "$ENV{VULKAN_SDK}/Bin/glslc.exe")
	else()
		set(GLSL_COMPILER "$ENV{VULKAN_SDK}/Bin32/glslc.exe")
	endif()
else()
	set(GLSL_COMPILER "$ENV{VULKAN_SDK}/bin/glslc")
endif()

set(PURPL_SHADER_SOURCES
	triangle.frag
	triangle.vert
)

foreach (CURRENT_FILE ${PURPL_SHADER_SOURCES})
	get_filename_component(CURRENT_FILE_NAME ${CURRENT_FILE} NAME)
	set(CURRENT_BINARY "${CMAKE_BINARY_DIR}/shaders/${CURRENT_FILE_NAME}.spv")
	set(CURRENT_FILE ${CMAKE_CURRENT_LIST_DIR}/${CURRENT_FILE})

	add_custom_command(
		OUTPUT ${CURRENT_BINARY}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders/"
		COMMAND ${GLSL_COMPILER} -o ${CURRENT_BINARY} ${CURRENT_FILE}
		DEPENDS ${CURRENT_FILE}
	)

	list(APPEND PURPL_SHADER_BINARIES ${CURRENT_BINARY})
endforeach(CURRENT_FILE)

add_custom_target(
	purpl_vulkan_shaders
	COMMENT "Compiling Vulkan shaders"
	DEPENDS ${PURPL_SHADER_BINARIES}
	SOURCES ${PURPL_SHADER_SOURCES}
)

add_custom_command(
	TARGET purpl_vulkan_shaders POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/demo/res/shaders"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_BINARY_DIR}/shaders" "${CMAKE_BINARY_DIR}/demo/res/shaders"
	DEPENDS purpl_vulkan_shaders
)
