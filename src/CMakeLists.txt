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
	../include/purpl/win32/graphics.h
	../include/purpl/win32/window.h
)

set(PURPL_WIN32_VULKAN_SOURCES
	win32/vulkan/debug.cc
	win32/vulkan/inst.cc
	win32/vulkan/util.cc
)

set(PURPL_WIN32_VULKAN_HEADERS
	../include/purpl/win32/vulkan/debug.h
	../include/purpl/win32/vulkan/inst.h
	../include/purpl/win32/vulkan/macro.h
	../include/purpl/win32/vulkan/util.h
	../include/purpl/win32/vulkan/vulkan_graphics.h
)

set(PURPL_X11_SOURCES
	x11/window.cc
)

set(PURPL_X11_HEADERS
	../include/purpl/x11/window.h
)

set(PURPL_SOURCES ${PURPL_COMMON_SOURCES})
set(PURPL_HEADERS ${PURPL_COMMON_HEADERS})

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(WINDOWS_EXPORT_ALL_SYMBOLS ON)
	list(APPEND PURPL_SOURCES ${PURPL_WIN32_SOURCES} ${PURPL_WIN32_${PURPL_GRAPHICS_API}_SOURCES})
	list(APPEND PURPL_HEADERS ${PURPL_WIN32_HEADERS} ${PURPL_WIN32_${PURPL_GRAPHICS_API}_HEADERS})
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	list(APPEND PURPL_SOURCES ${PURPL_X11_SOURCES} ${PURPL_X11_${PURPL_GRAPHICS_API}_SOURCES})
	list(APPEND PURPL_HEADERS ${PURPL_X11_HEADERS} ${PURPL_X11_${PURPL_GRAPHICS_API}_HEADERS})
endif()

add_library(purpl SHARED ${PURPL_SOURCES} ${PURPL_HEADERS})

target_link_libraries(purpl cglm imgui json-c ${Vulkan_LIBRARIES})

add_custom_command(TARGET purpl POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:purpl> ${CMAKE_BINARY_DIR}/demo)
