cmake_minimum_required(VERSION 3.10)

set(PURPL_DEMO_SOURCES
	main.cc
)

add_executable(purpl-demo ${PURPL_DEMO_SOURCES})

target_link_libraries(purpl-demo purpl)

add_custom_command(TARGET purpl-demo POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_NAME:purpl-demo> ${CMAKE_BINARY_DIR})
