# ue3-sdk-gen

(WIP) Unreal Engine 3 SDK generator for internal software, only tested on RL

## Key features

* generator runs internally or externally
* proper bitfield generation
* doesn't rely on your compiler to pad structs
* [fast af](https://youtu.be/6E7ZGCfruaw) (internal generates RL SDK in ~800ms)
* zero-copy UFunction calls
    * Using [CallableUFunction::Call](sdk/sdk_callableufunction.h)
* intuitive runtime UFunction hooking/blocking
    * Using [CallableUFunction::Hook*](sdk/sdk_callableufunction.h)
    * e.g. `UInteraction::Tick::HookPre([&](auto* params, UObject* obj, UFunction* fn, auto& ctx) { ... })`

## Caveats

* ### support will <u>&raquo; **NOT** &laquo;</u> be provided for targets other than Rocket League (for now)
    * please refrain from opening issues if you aren't targeting RL (unless the issue is target-agnostic).
* ### still wip, some things you want may be missing
    * accepting feature requests; open an issue
* only works on Windows
* only works for targets on x64 using wide-strings
* generated SDK needs C++23
    * haven't tested building with MSVC

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

* [src/config.h](src/config.h)
    * Update the offsets if they are out-of-date
* [src/processor.h](src/processor.h)
    * To change the output directory
* [src/unreal.h](src/unreal.h)
    * Change the unreal classes if necessary
* [sdk/sdk_unreal.h](sdk/sdk_unreal.h)
    * Again, change the unreal classes if necessary

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

### Usage

1. Run the generator
2. Copy the sdk_\* files and CMakeLists.txt file to your sdkgen output folder
3. copy the output folder into your project root dir
4. in your project's CMakeLists : `add_subdirectory` it and link to it in your target
5. initialize the sdk (resolves GObjects/GNames from offsets, sets up class/function cache)
    1. `rlsdk::rt().Initialize();`
6. if you want to use the function hooks you'll have to hook ProcessEvent yourself
    1. use `rlsdk::Rt::ProcessEvent_Hook` as the detour
    2. store the original in the `rlsdk::Rt::o_ProcessEvent` variable

### Credits

Thanks to TheFeckless
for [their work on their UE3 SDK generator](https://www.unknowncheats.me/forum/unreal-engine-3-a/71911-thefeckless-ue3-sdk-generator.html).
