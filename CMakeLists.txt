cmake_minimum_required(VERSION 3.10)

project(
	purpl
	VERSION 0.0.1
	LANGUAGES CXX)

# Building the demo should be the default
option(PURPL_BUILD_DEMO "Whether to build the demo" ON)

find_package(Vulkan)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(X11 REQUIRED)
endif()

if(${Vulkan_FOUND})
	set(VULKAN_LIBRARIES ${Vulkan_LIBRARIES})
	set(PURPL_GRAPHICS_API "VULKAN")
else()
	find_package(OpenGL)
endif()

include_directories(include ${Vulkan_INCLUDE_DIRS})

list(APPEND CMAKE_CXX_FLAGS "-O2 -DP_USE_${PURPL_GRAPHICS_API}_GFX") # MSVC does, in fact, support these arguments

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include_directories($ENV{UniversalCRTSdkDir}/include/ucrt)
	include_directories(
		$ENV{UniversalCRTSdkDir}/include/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/ucrt
	)
endif()

add_subdirectory(deps)

add_subdirectory(src)

if(${PURPL_BUILD_DEMO})
	add_subdirectory(demo)
endif()
