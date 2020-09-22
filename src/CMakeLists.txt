cmake_minimum_required(VERSION 3.10)

set(PURPL_COMMON_SOURCES
	app_info.cc
	inst.cc
	log.cc
	util.cc
)

set(PURPL_COMMON_HEADERS
	../include/purpl/app_info.h
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
	../include/purpl/window.h
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(WINDOWS_EXPORT_ALL_SYMBOLS ON)
	set(PURPL_SOURCES ${PURPL_COMMON_SOURCES} ${PURPL_WIN32_SOURCES})
	set(PURPL_HEADERS ${PURPL_COMMON_HEADERS} ${PURPL_WIN32_HEADERS})
endif()

add_library(purpl SHARED ${PURPL_SOURCES} ${PURPL_HEADERS})

target_link_libraries(purpl cglm imgui json-c ${Vulkan_LIBRARIES})

add_custom_command(TARGET purpl POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:purpl> ${CMAKE_BINARY_DIR})
