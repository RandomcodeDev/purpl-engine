cmake_minimum_required(VERSION 3.10)

set(PURPL_COMMON_SOURCES
	inst.cc
	log.cc
	util.cc
)

set(PURPL_WIN32_SOURCES
	win32/window.cc
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(WINDOWS_EXPORT_ALL_SYMBOLS ON)
	set(PURPL_SOURCES ${PURPL_COMMON_SOURCES} ${PURPL_WIN32_SOURCES})
endif()

add_library(purpl SHARED ${PURPL_SOURCES})

target_link_libraries(purpl cglm imgui json-c ${Vulkan_LIBRARIES})

add_custom_command(TARGET purpl POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_NAME:purpl> ${CMAKE_BINARY_DIR})
