//
// Created on 03-Dec-25.
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once

namespace config {

constexpr auto kProcessName = L"RocketLeague.exe"sv;

// constexpr auto kUseOffsets = true;
constexpr auto kGNamesOffset = 0x23D4D80u;
// constexpr auto kGNamesSignature = "75 ? E8 ? ? ? ? 48 C7 C7"sv;
constexpr auto kGObjectsOffset = 0x23D4DC8u;
// constexpr auto kGObjectsSignature = "E8 ? ? ? ? 8B 5D AF"sv;

constexpr auto kHeaderPrefix =
    "// ReSharper disable CppClassNeedsConstructorBecauseOfUninitializedMember\n#pragma once\n#pragma pack(push, 1)\n"sv;
constexpr auto kHeaderPostfix = "\n#pragma pack(pop)"sv;

}  // namespace config
