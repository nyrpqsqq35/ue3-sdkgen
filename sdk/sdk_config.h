//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once
#include <string_view>
#include <variant>
#include <array>
#include <bit>

namespace SDKGEN_NS {

struct OffsetConfig {
  struct Offset {
    uintptr_t offset;
    uintptr_t operator()() const;
  };
  struct Pattern {
    std::string_view pattern_str;
    uintptr_t operator()() const;
  };

  const std::string_view name;
  std::variant<Offset, Pattern> data;
  // PTR TO ANOTHER UINTPTR_T
  // can't be uintptr_t* because constexpr:)
  void* ptr_to_uintptr{};
  mutable uintptr_t value{};
};

// clang-format off
constexpr auto kOffsetConfigs = std::to_array<OffsetConfig>({
   { "GNames", OffsetConfig::Offset{0x23D4D80u}, &GNames },
   { "GObjects", OffsetConfig::Offset{0x23D4DC8u}, &GObjects },
});
// clang-format on

constexpr auto& kNamesOffset = kOffsetConfigs.at(0);
constexpr auto& kObjectsOffset = kOffsetConfigs.at(1);

}  // namespace SDKGEN_NS
