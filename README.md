# purpl-engine

## This repo is abandoned, for its replacement see [https://github.com/MobSlicer152/purpl-engine/blob/main]()

### Build instructions
You need the Vulkan SDK if you're building the Vulkan version of the engine, otherwise you need the headers for WGL/GLX and the other OpenGL headers. Assuming you have that stuff, you can run the following commands:
```bash
# You can avoid these if you add "--recursive" when you clone the repo
git submodule init
git submodule update

# Only Vulkan and OpenGL are supported at the moment
cmake -S. -Bbuild -DPURPL_GRAPHICS_API=VULKAN -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

___NOTE: In order to get the dependencies, you have to clone the repo with Git, otherwise you'll have to manually clone the individual repos into the `deps/` folder.___

___NOTE 2: Run `make`/`mingw32-make` in `deps/glew/auto` to generate the GLEW sources when you're building the OpenGL version of the engine___

___NOTE 3: Make sure to put the repo and your build directory somewhere with execute permissions when you're building on Linux___

### Supported graphics API names
This table has the values to set `PURPL_GRAPHICS_API` to that will be recognized:
| Graphics API | Value for `PURPL_GRAPHICS_API` |
| ------------ | ------------------------------ |
| OpenGL       | OPENGL                         |
| Vulkan       | VULKAN                         |

### Coding style
Adapt these guidelines to fit the circumstances: https://www.kernel.org/doc/html/latest/process/coding-style.html. Also see [`docs/style.md`](docs/style.md).

### API
See the `docs/api*.md` files (as of yet unwritten), and also look at the demo's code, as it has some useful comments. Also, I try my best to put explanatory comments stating what each bit of code does in the various functions, so look at those. [`docs/design.md`](docs/design.md) goes over its design.

### Other peoples' projects used in this one
This is a list of the other projects used/soon-to-be used in this project:
- [cglm](https://github.com/recp/cglm) is used for linear algebra stuff
- [GLEW](https://github.com/nigels-com/glew) is used for OpenGL loading
- [Dear ImGui](https://github.com/ocornut/imgui) is used for GUIs
- [json-c](https://github.com/json-c/json-c) is used for JSON parsing
- [stb](https://github.com/nothings/stb) is used for replacing `sprintf` and its related functions. See `stb_sprintf.h` from the link for information on extra features
