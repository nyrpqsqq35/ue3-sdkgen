//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once
#include <cstdint>
#include <string_view>
#include "sdk_hash_impl.h"

// ReSharper disable CppUnnamedNamespaceInHeaderFile
#ifndef SDKGEN_NS
// since this header is used in our sdkgen as well as the generated SDK, we default SDKGEN_NS empty
// so it can be used in sdkgen
#define SDKGEN_NS
#endif

namespace SDKGEN_NS {

template <uint64_t Seed = 0>
class StringHash {
 public:
  using value_type = uint64_t;
  using seed_type = decltype(Seed);

  constexpr StringHash() noexcept : value_(0) {}
  // ReSharper disable once CppNonExplicitConvertingConstructor
  constexpr StringHash(const value_type value) noexcept : value_(value) {}
  [[nodiscard]] constexpr value_type value() const noexcept { return value_; }

  [[nodiscard]] constexpr bool StrEqual(const std::string_view str) const {
    return BinEqual(str.data(), str.size());
  }

  [[nodiscard]] constexpr bool BinEqual(const void* data, const size_t data_len) const {
    return value_ == Calculate(data, data_len);
  }

  [[nodiscard]] constexpr static StringHash Calculate(const std::string_view str) {
    return Calculate(str.data(), str.size());
  }
  [[nodiscard]] constexpr static StringHash Calculate(const void* data, const size_t data_len) {
    return xxh64::hash(static_cast<const char*>(data), data_len, Seed);
  }

  constexpr bool operator==(const StringHash& other) const noexcept {
    return value_ == other.value_;
  }

 private:
  value_type value_;
};

};  // namespace SDKGEN_NS

template <SDKGEN_NS::StringHash<>::seed_type Seed>
struct std::hash<SDKGEN_NS::StringHash<Seed>> {
  std::size_t operator()(const SDKGEN_NS::StringHash<Seed>& sh) const noexcept {
    // maybe it should be return std::hash(sh.value())?
    return sh.value();
  }
};
