## API design
Overview: Explains the design of the API

### File structure
This diagram thingy explains it:

win32 _Windows-specific stuff_<br>
win32/graphics.h _Abstracts graphics APIs supported by Windows_<br>
macro.h _Macros for Windows things_<br>
window.h _The definition of the win32_window class, used for creating windows on Windows_<br>
win32/vulkan _Vulkan stuff_<br>
win32/vulkan/debug.h _Functions for debug mode (when the engine is compiled for the "Debug" target)_<br>
win32/vulkan/device.h _Functions for finding, querying, and using compute devices_<br>
win32/vulkan/inst.h _Definition of the win32_vulkan_inst class, used for graphics functions implemented using Vulkan_<br>
win32/vulkan/macro.h _Macros for Vulkan stuff_<br>
win32/vulkan/util.h _Utility functions, such as for retrieving a list of available extensions_<br>
win32/vulkan/vulkan_graphics.h _Included by the Windows graphics abstraction header_<br>
<br>
x11 WIP X11 stuff<br>
<br>
app_info.h _Definition of the app_info class, which provides an interface to use keys from a JSON file for basic program information_<br>
graphics.h _Abstracts graphics APIs supported by different platforms_<br>
inst.h _Definitions for the engine_inst class_<br>
log.h _Definitions for the logger class_<br>
macro.h _Various macros and definitions_<br>
purpl.h _A convenience header that should be included in programs wishing to use the API_<br>
types.h _Convenience types_<br>
window.h _Abstracts window creation across platforms_<br>