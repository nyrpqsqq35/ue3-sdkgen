//
// Created on 03-Dec-25.
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once
#include "sigscan.h"

namespace config {

// clang-format off
constexpr auto kProcessName = L"RocketLeague.exe"sv;

constexpr auto kUseOffsets = false;
constexpr auto kGNamesOffset = 0x23D4D80u;
constexpr auto kGNamesSignature = AOB_PATTERN(
  "49 8b dc 66 90 ?? ?? ?? ?? ?? ?? ?? 4c 63 04 18 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 48 8b 0d ?? ?? ?? ?? 4a 8b 0c c1"
);
constexpr auto kGNamesRelOp = 0x1d;
constexpr auto kGObjectsOffset = 0x23D4DC8u;
constexpr auto kGObjectsSignature = AOB_PATTERN(
  "90 48 63 4b 38 48 8b 05 ?? ?? ?? ?? 48 8b 0c c8 48 85 c9 75 ?? 48 89 75 f0 c7 45 ?? ?? ?? ?? ?? c7 45 ?? ?? ?? ?? ?? ba"
);
constexpr auto kGObjectsRelOp = 8;

constexpr auto kHeaderPrefix =
    "// ReSharper disable CppClassNeedsConstructorBecauseOfUninitializedMember\n#pragma once\n#pragma pack(push, 1)\n"sv;
constexpr auto kHeaderPostfix = "\n#pragma pack(pop)"sv;
// clang-format on

}  // namespace config
