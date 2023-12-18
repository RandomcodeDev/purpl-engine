## Purpl Engine

This is a game engine I'm making.

### Features

- Written in portable, modular C11
- xmake as the build system
- Diffuse lighting (note: Doom Guy's back is missing because of normal issues with the model that I got from a Google Drive
  folder of ripped models, and I didn't fix it because it was for testing)

  ![untextured diffuse lit Master Chief and Doom Guy models](https://randomcode.dev/programs/purpl_chief_doomguy_1.gif)
- Diffuse textured 3D models using custom simple formats about as efficient as OBJ and PNG

  ![textured Master Chief model](https://randomcode.dev/files/programs/purpl_chief_textured.png)
- Tools for converting to the engine's formats
- Decent Vulkan backend (and it's nearly 4000 lines long!)
- Partial DirectX 9 backend (for supporting operating systems older than I am like Windows XP)
- Partially implemented MSDF font support, which will allow for nice, crisp text rendering
- No DLL shenanigans (yet)

### Interesting stuff I guess

I've done my best to use [the coding style used for the Windows kernel](http://tenox.pdp-11.ru/os/winnt_xp/Documentation/NT_Design_Workbook/coding.doc)
because I think it's cool and also fairly readable, although kinda weird, impossible to match with an autoformatter,
and not compatible with Doxygen or anything.

The engine is made of these components:

- `common` - Shared common functions. Existance and some functions inspired by Quake 2.
- `platform` - Platform abstraction, handles the compiler(s) used for that platform but also OS functions,
also handles "video" (another Quake 2 idea sort of), through functions that hide most details
about the underlying window and such.
- `engine` - Currently contains a camera structure and function, a transform structure, and some ECS stuff.
- `rendersystem` (`engine/rendersystem`) - API-independant frontend for rendering, also inspired a bit by Quake 2.
- `rendersystem-vk` (`engine/rendersystem/vulkan`) - Vulkan render backend, more than a third of the lines of code in the engine.
- `rendersystem-dx` (`engine/rendersystem/directx`) - DirectX 12 backend, planned but currently empty.
- `rendersystem-dx9` (`engine/rendersystem/directx9`) - DirectX 9 backend, has fixed-function lighting but no texturing.
- `texture` (`util/texture`) - Texture format library. ZSTD compression, basically a header and pixels in ~~RGB~~, RGBA, or ~~depth
(32-bit float)~~ formats.
- `model` (`util/model`) - Model format library. Extremely primitive, like the texture format.
- `font` (`util/font`) - MSDF fonts using an atlas and JSON file generated by msdf-atlas-gen.

Other stuff:

- I use xmake, which is a really neat build system
- I heard that Doom Eternal uses thread jobs for everything so I hope to figure out how to do something like that.
- Everything is statically linked because DLL shenanigans are cool but not necessary.

### Cloning

The engine uses lots of submodules to reduce the amount of stuff that has to be
preinstalled, so it's probably advisable to clone recursively to avoid having to
check out the submodules separately:

```
git clone --depth 1 --recurse-submodules https://github.com/MobSlicer152/purpl-engine.git
```

If you forgot to clone recursively:

```
git submodule update --recursive --init
```

### Build instructions

Make sure to clone the submodules. See below for requirements.

```sh
# Build the tools first
pushd util
xmake build
popd

# Build the assets
pushd assets
python3 build.py -R ../tools -t ../util/build/<platform>/<arch>/release
popd

# To build for the GDK, run this before building
xmake f -p gdk --toolchain=msvc --as=llvm-as --vs=2022

# To build for Linux
xmake f -p linux --toolchain=clang

# xmake automatically uses the number of cores you have
xmake build

# To make project files
xmake project -k vsxmake # Visual Studio project
xmake project -k compile_commands # compile_commands.json for clangd in VS Code or Neovim
```

### Build requirements

You need [xmake](https://github.com/xmake-io/xmake).

- __Windows__:
  On Windows, you can build for the Microsoft Game Development Kit (`gdk`).
  Dependencies:
  - Windows 11
  - A recent Windows SDK
  - The [Microsoft GDK](https://github.com/microsoft/PGDK)
  - Visual Studio 2022 17.5

  For Windows development, I use Windows 11 and Visual Studio 2022.

- __Windows XP+__:
  You ~~can~~ could (no xmake port yet) also build for Windows XP and up, because I'm insane.
  Dependencies:
  - Windows
  - the Windows SDK
  - Visual Studio 2017 Windows XP tools (`v141_xp` toolset).

- __Linux/BSD__:
  Linux (and maybe BSD) also work.
  Dependencies:
  - XCB or Wayland headers and libraries for GLFW (`libx11-xcb-dev` or
    `libxcb-devel`, `wayland-client`, etc).
  - A recent version of Clang and the mold linker.

  The Linux video code uses GLFW for portability and correctness. The Linux build is usually tested
  with Arch Linux, Clang 16, and KDE Plasma or Hypr(land).

- __macOS__:
  This will probably also use GLFW. I want to add Metal rendering for it to actually work, though.

- __Console homebrew (PSP, Xbox 360, etc)__:
  This might happen eventually if I really feel like it.

- __Obscure Chinese and Russian CPU architectures__:
  I intend to have support for LoongArch and Elbrus E2K some day, for Loong Arch Linux and Astra Linux (any distro would work,
  those are just the ones I know of that support those architectures).

### Dependencies

I don't use all of these and probably don't need some of them, but it's still less than a Rust project (I would know, I've tried rewriting
this engine in Rust before). Many are only indirectly used to avoid the Vulkan SDK.

- [assimp](https://github.com/assimp/assimp) - Used in `meshtool` to convert meshes into the engine's format.
- [cglm](https://github.com/recp/cglm) - Used for math.
- [DirectX-Headers](https://github.com/Microsoft/DirectX-Headers) - Will be used for DirectX 12 backend.
- [flecs](https://github.com/SanderMertens/flecs) - Will be used for entities.
- [mimalloc](https://github.com/Microsoft/mimalloc) - Custom malloc, because every guide and game/engine says to use a different allocator,
  and this one is the most portable one that I found (it isn't actively used right now).
- msdf-atlas-gen - Generates funny font atlases.
- [shaderc](https://github.com/google/shaderc) - Google's GLSL/HLSL to SPIR-V compiler. Basically included to avoid needing the Vulkan SDK.
- [msdf-atlas-gen](https://github.com/Chlumsky/msdf-atlas-gen) - Used for generating font atlases.
- [stb](https://github.com/nothings/stb) - C header-only libraries used for image formats, dynamic lists, hash maps, and an alternate `snprintf`
  implementation.
- [volk](https://github.com/zeux/volk) - Vulkan loader.
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers) - Vulkan headers used to avoid the Vulkan SDK.
- [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - Vulkan memory allocation library.
