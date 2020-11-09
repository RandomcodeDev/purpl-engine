## API design
Overview: Explains the design of the API

### File structure
This diagram thingy explains it:

win32 _Windows-specific stuff_<br>
win32/macro.h _Macros for Windows things_<br>
win32/window.h _The definition of the win32_window class, used for creating windows on Windows_<br>
<br>
vulkan _Vulkan stuff_<br>
vulkan/buffer.h _Functions for passing and retreiving buffers that shaders use_<br>
vulkan/command.h _Functions for command buffers and command pools_<br>
vulkan/debug.h _Functions for debug mode (when the engine is compiled for the "Debug" target)_<br>
vulkan/framebuffer.h _Functions for framebuffers_<br>
vulkan/image.h _Functions for images_<br>
vulkan/inst.h _Definition of the vulkan_inst class, used for graphics functions implemented using Vulkan, and also a function to recreate the swapchain_<br>
vulkan/logical_device.h _Functions for creating logical devices_<br>
vulkan/macro.h _Macros for Vulkan stuff_<br>
vulkan/physical_device.h _Functions for locating and ranking compute devices_<br>
vulkan/pipeline.h _Functions for pipelines_<br>
vulkan/queuefamily.h _Functions for queue families_<br>
vulkan/spirv.h _Functions for manipulating shader modules_<br>
vulkan/swapchain.h _Functions for swapchain creation_<br>
vulkan/util.h _Utility functions, such as for retrieving a list of available extensions_<br>
vulkan/vulkan_graphics.h _Included by the graphics abstraction header_<br>
<br>
x11 _X11-specific stuff_<br>
x11/window.h _The definitions for the x11_window class, used for creating windows on Linux_<br>
<br>
app_info.h _Definition of the app_info class, which provides an interface to use keys from a JSON file for basic program information_<br>
graphics.h _Abstracts graphics APIs (i.e. `typedefs`, etc.)_<br>
inst.h _Definitions for the engine_inst class_<br>
log.h _Definitions for the logger class_<br>
macro.h _Various macros and definitions_<br>
purpl.h _A convenience header that should be included in programs wishing to use the API_<br>
types.h _Convenience types_<br>
window.h _Abstracts window creation across platforms_<br>

Then (as one might imagine) all the files in `src/` are just implementations.

### 

<sub>_Last updated 9/11/50 by MobSlicer152_</sub>
