cmake_minimum_required(VERSION 3.10)

project(
	purpl
	VERSION 0.0.1
	LANGUAGES CXX)

set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS)

if(${CMAKE_SYSTEM_NAME} MATCHES "Apple")
	message("macOS support is unavailable and also unlikely because my Hackintosh is unreliable and I will never give Apple my money.")
endif()

# Debug is the default
set(CMAKE_BUILD_TYPE Debug)

# Building the demo and using Vulkan should be the default
option(PURPL_BUILD_DEMO "Whether to build the demo" ON)
if (NOT DEFINED PURPL_GRAPHICS_API)
	set(PURPL_GRAPHICS_API "VULKAN")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(X11 REQUIRED)
endif()

if(${PURPL_GRAPHICS_API} STREQUAL "VULKAN")
	find_package(Vulkan REQUIRED)
elseif(${PURPL_GRAPHICS_API} STREQUAL "OPENGL")
	set(OpenGL_GL_PREFERENCE "GLVND")
	find_package(OpenGL REQUIRED)
endif()

include_directories(include)
if(${PURPL_GRAPHICS_API} STREQUAL "VULKAN")
	include_directories(${Vulkan_INCLUDE_DIRS})
elseif(${PURPL_GRAPHICS_API} STREQUAL "OPENGL")
	include_directories()
endif()

set(CMAKE_CXX_FLAGS "-O2 -Wall -DP_USE_${PURPL_GRAPHICS_API}_GFX") # MSVC does, in fact, support these arguments

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include_directories($ENV{UniversalCRTSdkDir}/include/ucrt 
		$ENV{UniversalCRTSdkDir}/include/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/ucrt
	)
endif()

add_subdirectory(deps)
add_subdirectory(src)
if(${PURPL_BUILD_DEMO})
	add_subdirectory(demo)
endif()
