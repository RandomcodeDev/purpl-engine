# purpl-engine

### Build instructions
You need the Vulkan SDK. Assuming you have it, you can run the following commands:
```bash
git submodule init
git submodule update

cmake -S. -Bbuild
cmake --build build
```

___NOTE: In order to get the dependencies, you have to clone the repo with Git, otherwise you have to manually clone the individual repos into the `deps/` folder.___

### API
Seeing as I just restarted this whole project from scratch, there's _very_ little to document. However, as the engine gains more features, they will be documented in the `docs/` folder, as well as demonstrated and explained in the demo, which can be found in the `demo/` folder. It's also likely that I'll eventually figure out Doxygen.
