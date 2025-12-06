//
// Created on 03-Dec-25.
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#include "util.h"

#define PTR_ADD_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) + (ULONG_PTR)(Offset)))
/**
 * Gets a pointer to the first process information structure in a buffer
 * returned by PhEnumProcesses().
 *
 * \param Processes A pointer to a buffer returned by PhEnumProcesses().
 */
#define PH_FIRST_PROCESS(Processes) ((PSYSTEM_PROCESS_INFORMATION)(Processes))

/**
 * Gets a pointer to the process information structure after a given structure.
 *
 * \param Process A pointer to a process information structure.
 *
 * \return A pointer to the next process information structure, or NULL if there
 * are no more.
 */
#define PH_NEXT_PROCESS(Process)                                                   \
  (((PSYSTEM_PROCESS_INFORMATION)(Process))->NextEntryOffset                       \
       ? (PSYSTEM_PROCESS_INFORMATION)PTR_ADD_OFFSET(                              \
             (Process), ((PSYSTEM_PROCESS_INFORMATION)(Process))->NextEntryOffset) \
       : NULL)

namespace util {

namespace internal {
void OpenFile(const std::string_view path, HANDLE& out_file) {
  out_file = ::CreateFileA(path.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
  if (out_file == INVALID_HANDLE_VALUE) {
    throw std::runtime_error("Failed to open file: CreateFileA failed");
  }
}
void CreateMapping(HANDLE file, HANDLE& out_mapping) {
  out_mapping =
      ::CreateFileMappingA(file, nullptr, PAGE_READONLY | SEC_IMAGE_NO_EXECUTE, 0, 0, nullptr);
  if (out_mapping == nullptr) {
    throw std::runtime_error("CreateFileMapping failed");
  }
}
void GetFileSize(HANDLE file, DWORD& hi_size, DWORD& lo_size) {
  lo_size = ::GetFileSize(file, &hi_size);
  if (lo_size == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
    throw std::runtime_error("GetFileSize failed");
  }
}
}  // namespace internal

std::pair<HANDLE, std::string> LocateProcess(const std::wstring_view process_name) {
  NTSTATUS res;
  std::vector<uint8_t> buf;
  DWORD buf_size = 0x4000;
  do {
    buf.resize(buf_size, 0);
    res =
        NtQuerySystemInformation(SystemExtendedProcessInformation, buf.data(), buf_size, &buf_size);
  } while (res == STATUS_BUFFER_TOO_SMALL || res == STATUS_INFO_LENGTH_MISMATCH);

  std::string out_path;
  if (NT_SUCCESS(res)) {
    PSYSTEM_PROCESS_INFORMATION process;

    process = PH_FIRST_PROCESS(buf.data());

    char full_image_path[512];
    do {
      if (process->ImageName.Length == 0) continue;
      if (!process_name._Equal(process->ImageName.Buffer)) continue;

      auto handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
                                reinterpret_cast<DWORD>(process->UniqueProcessId));
      if (handle != INVALID_HANDLE_VALUE) {
        // Resize out_path to fit most paths
        out_path.resize(1024, '\0');
        DWORD size = out_path.size();

        QueryFullProcessImageNameA(handle, 0, out_path.data(), &size);
        out_path.resize(size);
        return {handle, out_path};
      } else {
        LOG_TRACE("Couldn't open process during iteration");
      }
    } while ((process = PH_NEXT_PROCESS(process)));
  } else {
    throw std::runtime_error("NtQuerySystemInformation(SystemExtendedProcessInformation) failed");
  }

  throw std::runtime_error("Couldn't locate process");
}

Mapping MapFile(std::string_view path) {
  Mapping retval;
  HANDLE file, mapping;

  internal::OpenFile(path, file);
  internal::CreateMapping(file, mapping);
  internal::GetFileSize(file, retval.hi_size, retval.lo_size);

  retval.base = MapViewOfFile(mapping, FILE_MAP_READ,
                              0,  // hi offset
                              0,  // lo offset
                              0   // map entire file
  );
  if (retval.base == nullptr) throw std::runtime_error("MapViewOfFile failed");
  return retval;
}

// CSGOSimple
uintptr_t FindPattern(const Mapping& mapping, const char* signature) {
  static auto pattern_to_byte = [](const char* pattern) {
    auto bytes = std::vector<int>{};
    auto start = const_cast<char*>(pattern);
    auto end = const_cast<char*>(pattern) + strlen(pattern);

    for (auto current = start; current < end; ++current) {
      if (*current == '?') {
        ++current;
        if (*current == '?') ++current;
        bytes.push_back(-1);
      } else {
        bytes.push_back(strtoul(current, &current, 16));
      }
    }
    return bytes;
  };
  auto patternBytes = pattern_to_byte(signature);

  auto s = patternBytes.size();
  auto d = patternBytes.data();

  const auto base = reinterpret_cast<const uint8_t*>(mapping.base);

  for (auto i = 0ul; i < mapping.size; ++i) {
    bool found = true;
    for (auto j = 0ul; j < s; ++j) {
      if (base[i + j] != d[j] && d[j] != -1) {
        found = false;
        break;
      }
    }
    if (found) {
      return (uintptr_t)&base[i] - (uintptr_t)mapping.base;
    }
  }

  return NULL;
}

uintptr_t GetProcessBaseAddress(const HANDLE process) {
  PROCESS_BASIC_INFORMATION pbi;

  NTSTATUS res =
      NtQueryInformationProcess(process, ProcessBasicInformation, &pbi, sizeof(pbi), nullptr);
  if (!NT_SUCCESS(res)) {
    throw std::runtime_error("NtQueryInformationProcess(ProcessBasicInformation) failed");
  }

  uintptr_t base_address;
  if (!ReadProcessMemory(process,
                         (void*)((uintptr_t)pbi.PebBaseAddress + offsetof(PEB, ImageBaseAddress)),
                         &base_address, sizeof(base_address), nullptr)) {
    throw std::runtime_error("ReadProcessMemory failed");
  }
  return base_address;
}

}  // namespace util
