cmake_minimum_required(VERSION 3.10)

set(PURPL_COMMON_SOURCES
	app_info.cc
	inst.cc
	log.cc
	util.cc
)

set(PURPL_COMMON_HEADERS
	../include/purpl/app_info.h
	../include/purpl/graphics.h
	../include/purpl/inst.h
	../include/purpl/log.h
	../include/purpl/macro.h
	../include/purpl/purpl.h
	../include/purpl/types.h
	../include/purpl/util.h
	../include/purpl/window.h
)

set(PURPL_WIN32_SOURCES 
	win32/window.cc
)

set(PURPL_WIN32_HEADERS
	../include/purpl/win32/macro.h
	../include/purpl/win32/window.h
)

set(PURPL_WIN32_OPENGL_SOURCES
	win32/opengl.cc
)

set(PURPL_WIN32_OPENGL_HEADERS
	../include/purpl/win32/opengl.h
)

set(PURPL_VULKAN_SOURCES
	vulkan/buffer.cc
	vulkan/command.cc
	vulkan/debug.cc
	vulkan/framebuffer.cc
	vulkan/image.cc
	vulkan/inst.cc
	vulkan/logical_device.cc
	vulkan/pipeline.cc
	vulkan/physical_device.cc
	vulkan/queuefamily.cc
	vulkan/spirv.cc
	vulkan/surface.cc
	vulkan/swapchain.cc
	vulkan/util.cc
)

set(PURPL_VULKAN_HEADERS
	../include/purpl/vulkan/buffer.h
	../include/purpl/vulkan/command.h
	../include/purpl/vulkan/debug.h
	../include/purpl/vulkan/framebuffer.h
	../include/purpl/vulkan/image.h
	../include/purpl/vulkan/inst.h
	../include/purpl/vulkan/logical_device.h
	../include/purpl/vulkan/macro.h
	../include/purpl/vulkan/pipeline.h
	../include/purpl/vulkan/physical_device.h
	../include/purpl/vulkan/queuefamily.h
	../include/purpl/vulkan/spirv.h
	../include/purpl/vulkan/surface.h
	../include/purpl/vulkan/swapchain.h
	../include/purpl/vulkan/util.h
	../include/purpl/vulkan/vulkan_graphics.h
)

set(PURPL_X11_SOURCES
	x11/window.cc
)

set(PURPL_X11_HEADERS
	../include/purpl/x11/window.h
)

set(PURPL_SOURCES ${PURPL_COMMON_SOURCES})
set(PURPL_HEADERS ${PURPL_COMMON_HEADERS})

if(${PURPL_GRAPHICS_API} STREQUAL "VULKAN")
	list(APPEND PURPL_SOURCES ${PURPL_VULKAN_SOURCES})
	list(APPEND PURPL_HEADERS ${PURPL_VULKAN_HEADERS})
	source_group("Vulkan" FILES ${PURPL_VULKAN_SOURCES} ${PURPL_VULKAN_HEADERS})
endif()


if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(WINDOWS_EXPORT_ALL_SYMBOLS ON)
	list(APPEND PURPL_SOURCES ${PURPL_WIN32_SOURCES})
	list(APPEND PURPL_HEADERS ${PURPL_WIN32_HEADERS})
	source_group("Windows" FILES ${PURPL_WIN32_SOURCES} ${PURPL_WIN32_HEADERS})
	
	if(${PURPL_GRAPHICS_API} STREQUAL "OPENGL")
		list(APPEND PURPL_SOURCES ${PURPL_WIN32_OPENGL_SOURCES})
		list(APPEND PURPL_SOURCES ${PURPL_WIN32_OPENGL_HEADERS})
		source_group("Windows/OpenGL" FILES ${PURPL_WIN32_OPENGL_SOURCES} ${PURPL_WIN32_OPENGL_HEADERS})
	endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	list(APPEND PURPL_SOURCES ${PURPL_X11_SOURCES})
	list(APPEND PURPL_HEADERS ${PURPL_X11_HEADERS})
	source_group("Linux" FILES ${PURPL_X11_SOURCES} ${PURPL_X11_HEADERS})
endif()

add_library(purpl SHARED ${PURPL_SOURCES} ${PURPL_HEADERS})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(purpl X11)
endif()

target_link_libraries(purpl cglm imgui json-c)

if(${PURPL_GRAPHICS_API} STREQUAL "VULKAN")
	target_link_libraries(purpl ${Vulkan_LIBRARIES})
endif()

if(${PURPL_GRAPHICS_API} STREQUAL "VULKAN")
	add_subdirectory(vulkan/shaders)
	add_dependencies(purpl purpl_vulkan_shaders)
else()
	add_custom_command(
	TARGET purpl
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different vulkan/shaders/triangle.vert
	vulkan/shaders/triangle.frag ${CMAKE_BINARY_DIR}/demo/res/shaders
)
endif()

# Copy the library for the engine so the demo can run
if(${PURPL_BUILD_DEMO})
	add_custom_command(
		TARGET purpl
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:purpl>
		${CMAKE_BINARY_DIR}/demo
	)
endif()
