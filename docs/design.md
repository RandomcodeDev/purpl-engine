## API design
Overview: Explains the design of the API

### File structure
This diagram thingy explains it:
include
|-------- purpl _API headers_
          |----- win32 _Windows-specific stuff_
          |      |----- graphics.h _Abstracts graphics APIs supported by Windows_
          |      |----- macro.h _Macros for Windows things_
          |      |----- window.h _The definition of the win32_window class, used for creating windows on Windows_
          |      |----- vulkan _Vulkan stuff_
          |             |------ debug.h _Functions for debug mode (when the engine is compiled for the "Debug" target)_
          |             |------ device.h _Functions for finding, querying, and using compute devices_
          |             |------ inst.h _Definition of the win32_vulkan_inst class, used for graphics functions implemented using Vulkan_
          |             |------ macro.h _Macros for Vulkan stuff_
          |             |------ util.h _Utility functions, such as for retrieving a list of available extensions_
          |             |------ vulkan_graphics.h _Included by the Windows graphics abstraction header_
          |
          |----- x11 WIP X11 stuff
          |
          |----- app_info.h _Definition of the app_info class, which provides an interface to use keys from a JSON file for basic program information_
          |----- graphics.h _Abstracts graphics APIs supported by different platforms_