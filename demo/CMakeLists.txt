cmake_minimum_required(VERSION 3.10)

set(PURPL_DEMO_SOURCES
	main.cc
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(PURPL_DEMO_SOURCES ${PURPL_DEMO_SOURCES} purpl-demo.rc)
endif()

add_executable(purpl-demo ${PURPL_DEMO_SOURCES})

target_link_libraries(purpl-demo purpl)

add_custom_command(TARGET purpl-demo POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_NAME:purpl-demo> ${CMAKE_CURRENT_LIST_DIR}/res/purpl.ico ${CMAKE_BINARY_DIR})
