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

set(IMGUI_IMPL_SOURCES
	imgui/examples/imgui_impl_vulkan.cpp
)

set(IMGUI_IMPL_HEADERS
	imgui/examples/imgui_impl_vulkan.h
)

add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS} ${IMGUI_IMPL_SOURCES} ${IMGUI_IMPL_HEADERS})

target_link_libraries(imgui ${Vulkan_LIBRARIES})

add_subdirectory(cglm)
add_subdirectory(json-c)

include_directories(cglm/include imgui imgui/examples ${CMAKE_CURRENT_BINARY_DIR}/deps/json-c/ json-c/include)
