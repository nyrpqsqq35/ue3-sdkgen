//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#include "sdk.h"
#include "sdk_config.h"

#include <format>
#include <fstream>
#include <thread>

namespace SDKGEN_NS {

#if defined(_WIN64) || defined(_WIN32)
// avoid dependence on Windows headers
uintptr_t GetImageBaseAddress() noexcept {
#ifdef _WIN64
  constexpr auto kImageBaseAddressOffset = 0x10;
  const auto peb = __readgsqword(0x60);
#else
  constexpr auto kImageBaseAddressOffset = 0x08;
  const auto peb = __readfsdword(0x30);
#endif
  return *reinterpret_cast<uintptr_t*>(peb + kImageBaseAddressOffset);
}
#else
static_assert(false, "You need to implement GetImageBaseAddress for your target platform.");
#endif

uintptr_t OffsetConfig::Offset::operator()() const { return GetImageBaseAddress() + offset; }
uintptr_t OffsetConfig::Pattern::operator()() const {
  throw std::runtime_error("not implemented yet");
}

void Initialize() {
  for (const auto& [name, data, value] : kOffsetConfigs) {
    value = std::visit([](auto&& a) -> uintptr_t { return a(); }, data);
    if (value == 0) {
      throw std::runtime_error(std::format("failed to resolve {}", name));
    }
    // LOG_INFO("{} resolved to {:#016x}", name, value);
  }

  GObjects = reinterpret_cast<TArray<UObject*>*>(kObjectsOffset.value);
  GNames = reinterpret_cast<TArray<FNameEntry*>*>(kNamesOffset.value);
}

}  // namespace SDKGEN_NS
