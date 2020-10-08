set(IMGUI_SOURCES
	imgui/imgui.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_widgets.cpp
)

set(IMGUI_HEADERS
	imgui/imconfig.h
	imgui/imgui.h
	imgui/imgui_internal.h
	imgui/imstb_rectpack.h
	imgui/imstb_textedit.h
	imgui/imstb_truetype.h
)

set(IMGUI_IMPL_OPENGL_SOURCES
	imgui/examples/imgui_impl_opengl3.cpp
)

set(IMGUI_IMPL_OPENGL_HEADERS
	imgui/examples/imgui_impl_opengl3.h
)

set(IMGUI_IMPL_VULKAN_SOURCES
	imgui/examples/imgui_impl_vulkan.cpp
)

set(IMGUI_IMPL_VULKAN_HEADERS
	imgui/examples/imgui_impl_vulkan.h
)

add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS} ${IMGUI_IMPL_${PURPL_GRAPHICS_API}_SOURCES} ${IMGUI_IMPL_${PURPL_GRAPHICS_API}_HEADERS})
target_link_libraries(imgui ${${PURPL_GRAPHICS_API}_LIBRARIES})

add_subdirectory(cglm)
add_subdirectory(json-c)

include_directories(cglm/include imgui imgui/examples ${CMAKE_CURRENT_BINARY_DIR}/deps/json-c/ json-c/include)
