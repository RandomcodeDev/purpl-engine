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

### Coding style
Adapt these guidelines to fit the circumstances: https://www.kernel.org/doc/html/latest/process/coding-style.html. Also see [`docs/style.md`](docs/style.md).

### API
See the `docs/api*.md` files, and also look at the demo's code, as it has some useful comments. Also, I try my best to put explanatory comments stating what each bit of code does in the various functions, so look at those. [`docs/design.md`](docs/design.md) goes over its design.