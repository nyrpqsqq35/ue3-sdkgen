//
// Created on 03-Dec-25.
// Copyright (c) 2025. All rights reserved.
//

#pragma once

namespace util {

struct Mapping {
  void* base = nullptr;
  union {
    struct {
      DWORD lo_size, hi_size;
    };
    uint64 size = 0;
  };
};

std::pair<HANDLE, std::string> LocateProcess(std::wstring_view process_name);
Mapping MapFile(std::string_view path);
uintptr_t FindPattern(const Mapping& mapping, const char* signature);
uintptr_t GetProcessBaseAddress(HANDLE process);

}  // namespace util
