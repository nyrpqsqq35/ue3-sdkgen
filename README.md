# ue3-sdk-gen

(WIP) Unreal Engine 3 SDK generator, only tested on RL

## Key features

* generator runs internally or externally
* proper bitfield generation
* [fast af](https://youtu.be/6E7ZGCfruaw) (internal generates RL sdk in ~800ms)

## Caveats

* only works on Windows
* only works for targets on x64 using wide-strings
* still wip, some things you want may be missing

## Building

### Prerequisites

* [Vcpkg](https://github.com/Microsoft/vcpkg)
* [CMake](https://cmake.org/)
* [Ninja](https://ninja-build.org/)
* [LLVM/Clang 20+](https://github.com/llvm/llvm-project/releases)

### Compiling

Ensure you have installed the build prerequisites!!

Ensure submodules are cloned!!! (`git clone --recursive` or `git submodule update --init --recursive` after cloning)

You will need to modify the following files:

- [src/config.h](src/config.h)
    - Update the offsets if they are out-of-date
- [src/processor.h](src/processor.h)
    - To change the output directory

```pwsh
# Configure CMake project
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:\\path\\to\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake

# Compile
cmake --build build --config Release

# Show compiled binaries
dir build/out/

# ue3_sdk_gen_internal.dll
#   - Inject into the game to generate the SDK
# ue3_sdk_gen.exe
#   - Run in terminal to generate the SDK
```

