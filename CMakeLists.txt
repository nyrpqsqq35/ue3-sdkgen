cmake_minimum_required(VERSION 3.31)
set(VCPKG_TARGET_TRIPLET "x64-win-llvm-lto-static-md")
set(VCPKG_HOST_TRIPLET "${VCPKG_TARGET_TRIPLET}")
project(ue3_sdk_gen)

set(CMAKE_CXX_STANDARD 26)

add_subdirectory(third_party/phnt)
find_package(absl CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

set_target_properties(spdlog::spdlog_header_only PROPERTIES INTERFACE_COMPILE_OPTIONS "")

option(SDKGEN_IN_TREE_SDK "Set to ON if you want to edit the runtime SDK stuff" OFF)
if (SDKGEN_IN_TREE_SDK)
    add_subdirectory(sdk)
endif ()

set(SRCS src/main.cc
        src/pch.h
        src/bridge.h
        src/unreal.cc
        src/unreal.h
        src/unreal_enums.h
        src/util.cc
        src/util.h
        src/config.h
        src/processor.cc
        src/processor.h
        src/processor_classes.cc
        src/processor_classes.h
        src/processor_consts.cc
        src/processor_consts.h
        src/processor_enums.cc
        src/processor_enums.h
        src/processor_structs.cc
        src/processor_structs.h
        src/bridge_impl_internal.h
        src/bridge_impl_external.h
        src/bridge.cc)
set(LINK_LIBS absl::flat_hash_map absl::flat_hash_set absl::node_hash_map absl::node_hash_set
        absl::time absl::status absl::statusor absl::strings
        spdlog::spdlog_header_only magic_enum::magic_enum
        phnt::phnt
)
add_executable(ue3_sdk_gen ${SRCS})
target_link_libraries(ue3_sdk_gen PRIVATE ${LINK_LIBS})
set_target_properties(ue3_sdk_gen PROPERTIES
        PRECOMPILE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/out"
)
add_library(ue3_sdk_gen_internal SHARED ${SRCS})
target_link_libraries(ue3_sdk_gen_internal PRIVATE ${LINK_LIBS})
target_compile_definitions(ue3_sdk_gen_internal PRIVATE
        SDKGEN_INTERNAL
)
set_target_properties(ue3_sdk_gen_internal PROPERTIES
        PRECOMPILE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/out"
)
# HACK
target_include_directories(ue3_sdk_gen PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdk")
target_include_directories(ue3_sdk_gen_internal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdk")