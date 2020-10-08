
cmake_minimum_required(VERSION 3.10)

project(purpl VERSION 0.0.1 LANGUAGES CXX)

# Building the demo should be the default
option(PURPL_BUILD_DEMO "Whether to build the demo" ON)

find_package(Vulkan REQUIRED)
find_package(X11 REQUIRED)

if (${Vulkan_FOUND})
	set(VULKAN_LIBRARIES ${Vulkan_LIBRARIES})
endif()

include_directories(include ${Vulkan_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DP_USE_${PURPL_GRAPHICS_API}_GFX") # MSVC does, in fact, support these arguments

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include_directories($ENV{UniversalCRTSdkDir}/include/ucrt)
	include_directories($ENV{UniversalCRTSdkDir}/include/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/ucrt)
endif()

add_subdirectory(deps)

add_subdirectory(src)

if (${PURPL_BUILD_DEMO})
	add_subdirectory(demo)
	set(VS_STARTUP_PROJECT purpl-demo) # Tell Visual Studio how to run the demo in the event that it gets built
	set_property(TARGET purpl-demo PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
