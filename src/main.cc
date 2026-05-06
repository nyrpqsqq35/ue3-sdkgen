//
// Created on 03-Dec-25.
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#include "build_config.h"
#include <stdexcept>
#include "bridge.h"
#include "util.h"
#include "config.h"
#include "processor_classes.h"
#include "processor_consts.h"
#include "processor_enums.h"
#include "processor_structs.h"
#include "unreal.h"
#include "sigscan.h"
#include <spdlog/fmt/bin_to_hex.h>
#include <magic_enum/magic_enum_all.hpp>
#include <simdjson.h>

// #define ENABLE_DUMP_NAMES
// #define ENABLE_DUMP_OBJECTS
namespace {

using namespace bridge;
uintptr_t base_address = 0;

template <typename T>
uintptr_t derel(const util::Mapping& map, const ptrdiff_t offset, const ptrdiff_t diff) {
  T rel_value = *(T*)((uintptr_t)map.base + offset + diff);
  uintptr_t abs_value = ((uintptr_t)offset + diff + sizeof(T)) + rel_value;
  return abs_value;
}

void FindAndDumpGNames(const util::Mapping& mapping) {
  auto offset = config::kGNamesOffset;
  if (!config::kUseOffsets) {
    if (auto located =
            avx2_strstr_anysize((byte*)mapping.base, mapping.size, config::kGNamesSignature);
        located == std::string::npos) {
      throw std::runtime_error("gnames signature is bad");
    } else {
      offset = derel<uint32>(mapping, located, config::kGNamesRelOp);
    }
  }

  const Pointer<int> v0(base_address + offset);
  if (!v0.IsValid()) {
    throw std::runtime_error("Couldn't locate GNames");
  }
  LOG_INFO("GNames @ {:#16x}", v0.get());
  GNames.Set(v0.get());
  const auto vec = GNames->Fetch();

#ifdef ENABLE_DUMP_NAMES
  std::wofstream names_file("./names.txt", std::ios_base::out);
#endif
  for (auto& fname : vec) {
    if (!fname.IsValid()) continue;
#ifdef ENABLE_DUMP_NAMES
    names_file << L"Name[" << std::setw(6) << std::setfill(L'0') << std::dec << fname->Index
               << "] ";

    names_file << fname->Name;

    names_file << " @ " << std::showbase << std::setw(10) << std::setfill(L'0') << std::hex
               << fname.get() << "\n";
#endif
    FNameIndexToString.insert({fname->Index, fname->Name});
  }
#ifdef ENABLE_DUMP_NAMES
  names_file << std::endl;
  names_file.close();
#endif
}
void FindAndDumpGObjects(const util::Mapping& mapping) {
  auto offset = config::kGObjectsOffset;
  if (!config::kUseOffsets) {
    if (auto located =
            avx2_strstr_anysize((byte*)mapping.base, mapping.size, config::kGObjectsSignature);
        located == std::string::npos) {
      throw std::runtime_error("gobjects signature is bad");
    } else {
      offset = derel<uint32>(mapping, located, config::kGObjectsRelOp);
    }
  }

  const Pointer<int> v4(base_address + offset);
  if (!v4.IsValid()) {
    throw std::runtime_error("Couldn't locate GObjects");
  }

  LOG_INFO("GObjects @ {:#16x}", v4.get());
  GObjects.Set(v4.get());
  const auto vec = GObjects->Fetch();

#ifdef ENABLE_DUMP_OBJECTS
  std::wofstream objects_file("./object.txt", std::ios_base::out);
#endif
  for (auto& object : vec) {
    if (!object.IsValid()) continue;

#ifdef ENABLE_DUMP_OBJECTS
    objects_file << L"Object[" << std::setw(6) << std::setfill(L'0') << std::dec
                 << object->object_internal_integer << L"] ";
    objects_file << object->GetFullName().c_str();
    objects_file << " @ " << std::showbase << std::setw(10) << std::setfill(L'0') << std::hex
                 << object.get();
    auto broken_flags = magic_enum::enum_flags_name(object->object_flags);
    objects_file << " .... " << std::wstring(broken_flags.begin(), broken_flags.end()) << "\n";
#endif

    StringToClass.insert({object->GetFullName(), Pointer<UClass>(object.get())});
  }
#ifdef ENABLE_DUMP_OBJECTS
  objects_file << std::endl;
  objects_file.close();
#endif
  // Pointer<TArray<Pointer<FNameEntry>>> gobjects(ptr_to_gobjects.get());
  // DLOG(INFO) << "GObjects @ " << std::hex << ptr_to_gobjects.get();
}
}  // namespace

template <simdjson::constevalutil::fixed_string Name, class T>
void AppendKeyContainer(simdjson::builder::string_builder& builder,
                        const std::vector<T>& container) {
  builder.escape_and_append_with_quotes<Name>();
  builder.append_colon();
  builder.start_array();
  if (container.size() > 0) {
    builder.append(container.front());
    // if (container.size() > 1)
    for (auto it = container.cbegin() + 1; it != container.cend(); ++it) {
      builder.append_comma();
      const T& v = *it;
      builder.append<const T&>(v);
    }
  }
  builder.end_array();
}

namespace simdjson {

template <typename builder_type>
void tag_invoke(serialize_tag, builder_type& builder,
                const processor::Package::ObjectProperty& prop) {
  builder.start_object();
  builder.append_key_value("name", prop.name);
  builder.append_comma();
  builder.append_key_value("flags", prop.flags);
  builder.append_comma();
  builder.append_key_value("offset", prop.offset);
  builder.append_comma();
  builder.append_key_value("type", prop.type);
  if (prop.num_elements != 1) {
    builder.append_comma();
    builder.append_key_value("num_elements", prop.num_elements);
  }
  if (prop.mask.has_value()) {
    builder.append_comma();
    builder.append_key_value("mask", prop.mask.value());
  }
  builder.end_object();
}

template <typename builder_type>
void tag_invoke(serialize_tag, builder_type& builder, const processor::Package::JsonObject& obj) {
  builder.start_object();

  builder.append_key_value("name", obj.name);
  builder.append_comma();
  builder.append_key_value("size", obj.size);
  builder.append_comma();

  builder.append_key_value("full_name", obj.ptr->GetFullName());
  builder.append_comma();
  builder.append_key_value("flags", std::to_underlying(obj.ptr->object_flags));
  builder.append_comma();

  if (obj.inheritance) {
    builder.append_key_value("inheritance", obj.inheritance);
    builder.append_comma();
  }

  builder.append_key_value("props", obj.props);

  builder.end_object();
}

template <typename builder_type>
void tag_invoke(serialize_tag, builder_type& builder, const processor::Package::JsonEnum& e) {
  builder.start_object();
  builder.append_key_value("name", e.name);
  builder.append_comma();
  builder.append_key_value("values", e.values);
  builder.end_object();
}

template <typename builder_type>
void tag_invoke(serialize_tag, builder_type& builder, const processor::Package::JsonConst& e) {
  builder.start_object();
  builder.append_key_value("name", e.name);
  builder.append_comma();
  builder.append_key_value("value", e.value);
  builder.end_object();
}

template <typename builder_type>
void tag_invoke(serialize_tag, builder_type& builder,
                const std::pair<std::string, std::string>& e) {
  builder.start_array();
  builder.escape_and_append_with_quotes(e.first);
  builder.append_comma();
  builder.escape_and_append_with_quotes(e.second);
  builder.end_array();
}

}  // namespace simdjson

void GenerateSDK() {
  auto [process_, image_path] = util::LocateProcess(config::kProcessName);
  bridge::process = process_;
  const auto mapping = util::MapFile(image_path);
  base_address = util::GetProcessBaseAddress(process);
  LOG_INFO("Base address: {:#016x}", base_address);

  FindAndDumpGNames(mapping);
  FindAndDumpGObjects(mapping);

  LOG_INFO("Processing objects");
  auto objects = GObjects->Fetch();
  for (auto& obj : objects) {
    if (!obj.IsValid()) continue;

    if (obj->IsA("Class Core.Class")) {
      if (auto package = obj->GetPackageObject(); !processor::HasPackageBeenCreated(package)) {
        processor::CreatePackage(package);
      }
    }

    if (obj->IsA("Class Core.Const")) {
      processor::consts::ProcessConst(obj);
    } else if (obj->IsA("Class Core.Enum")) {
      processor::enums::ProcessEnum(obj);
    } else if (obj->IsA("Class Core.Class")) {
      processor::classes::ProcessClass(obj);
    } else if (obj->IsA("Class Core.Function")) {
      // This must come before Struct or else we will process Functions as structs
      // LOG(INFO) << "Function: " << obj->GetFullName();
    } else if (obj->IsA("Class Core.State")) {
      // same as above
    } else if (obj->IsA("Class Core.Struct")) {
      processor::structs::ProcessStruct(obj);
    }
  }

  for (const auto& p : processor::GetAllPackages()) {
    LOG_INFO("Writing structs to disk");
    std::ofstream json_file(processor::sdk_path / (p.first->name.ToString() + ".json"),
                            std::ios_base::out);
    std::ofstream structs_file(processor::sdk_path / (p.first->name.ToString() + "_structs.h"),
                               std::ios_base::out);

    structs_file << config::kHeaderPrefix;

    for (const auto& st : p.second.generated_structs) {
      structs_file << st.generated_cpp << "\n";
    }
    structs_file << config::kHeaderPostfix << std::endl;
    structs_file.close();

    LOG_INFO("Writing classes, consts, enums, and params to disk");
    std::ofstream classes_file(processor::sdk_path / (p.first->name.ToString() + "_classes.h"),
                               std::ios_base::out);
    std::ofstream params_file(processor::sdk_path / (p.first->name.ToString() + "_params.h"),
                              std::ios_base::out);
    std::ofstream classes_cc_file(processor::sdk_path / (p.first->name.ToString() + "_classes.cc"),
                                  std::ios_base::out);
    classes_file << config::kHeaderPrefix;
    params_file << config::kHeaderPrefix;
    classes_cc_file << "#include \"sdk.h\"\n#pragma pack(push, 1)\n";

    for (const auto& st : p.second.generated_classes) {
      classes_file << st.generated_cpp << "\n";
      if (!st.generated_header.empty()) params_file << st.generated_header << "\n";
      if (!st.generated_source.empty()) classes_cc_file << st.generated_source << "\n";
    }

    classes_file << config::kHeaderPostfix << std::endl;
    params_file << config::kHeaderPostfix << std::endl;
    classes_cc_file << config::kHeaderPostfix << std::endl;

    classes_file.close();
    params_file.close();
    classes_cc_file.close();

    if (!p.second.generated_macros.empty()) {
      std::ofstream macros_file(processor::sdk_path / (p.first->name.ToString() + "_macros.h"),
                                std::ios_base::out);
      macros_file << config::kHeaderPrefix;
      for (const auto& st : p.second.generated_macros) {
        macros_file << st << "\n";
      }
      macros_file << config::kHeaderPostfix;
      macros_file.close();
    }

    // Create JSON object
    simdjson::builder::string_builder sb;
    sb.start_object();
    sb.append_key_value("name", p.first->GetFullName());
    sb.append_comma();
    AppendKeyContainer<"structs">(sb, p.second.json_structs);
    sb.append_comma();
    AppendKeyContainer<"classes">(sb, p.second.json_classes);
    sb.append_comma();
    AppendKeyContainer<"enums">(sb, p.second.json_enums);
    sb.append_comma();
    AppendKeyContainer<"consts">(sb, p.second.json_consts);
    sb.end_object();
    json_file.write(sb.c_str(), sb.size());
    json_file.close();
  }

  LOG_INFO("Writing verifier.cc");
  std::ofstream verifier_file(processor::sdk_path / "verifier.cc", std::ios_base::out);
  verifier_file << "#include \"sdk.h\"\n";

  verifier_file << "#pragma clang diagnostic push\n";
  verifier_file << "#pragma clang diagnostic ignored \"-Winvalid-offsetof\"\n";
  for (const auto& st : processor::VerifierStatements) {
    verifier_file << st << "\n";
  }
  verifier_file << "#pragma clang diagnostic pop\n";
  verifier_file.close();
}

#ifdef SDKGEN_INTERNAL
#define MboxFmtWFlags(Flags, Fmt, ...) \
  MessageBoxA(nullptr, std::format(Fmt, __VA_ARGS__).c_str(), "meow", Flags);
#define MboxFmt(Fmt, ...) MboxFmtWFlags(MB_OK, Fmt, __VA_ARGS__)
#define MboxFmtErr(Fmt, ...) MboxFmtWFlags(MB_ICONERROR | MB_OK, Fmt, __VA_ARGS__)

void MainThread(HMODULE module) {
#ifdef CI_MESSAGE
  MboxFmt("UE3-SDK-GEN v{}+{} started\n{}", PROJECT_VERSION, GIT_HASH, CI_MESSAGE);
#else
  MboxFmt("UE3-SDK-GEN v{}+{} started", PROJECT_VERSION, GIT_HASH);
#endif
  const auto start_time = absl::Now();

  try {
    GenerateSDK();
  } catch (std::exception& ex) {
    MboxFmtErr("Uncaught error during SDK generation: {}", ex.what());
  }

  const auto end_time = absl::Now();
  const auto duration = end_time - start_time;
  MboxFmt("Generator is complete in {}", absl::FormatDuration(duration));
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
  // Perform actions based on the reason for calling.
  switch (reason) {
    case DLL_PROCESS_ATTACH:
      spdlog::set_level(spdlog::level::trace);
      CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, instance, 0, 0);
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      if (reserved != nullptr) {
        break;  // do not do cleanup if process termination scenario
      }
    default:
      break;
  }
  return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

#undef MboxFmtWFlags
#undef MboxFmt
#undef MboxFmtErr
#else

int main() {
  spdlog::set_level(spdlog::level::trace);

  const auto start_time = absl::Now();
  LOG_INFO("UE3-SDK-GEN v{}+{}", PROJECT_VERSION, GIT_HASH);
#ifdef CI_MESSAGE
  LOG_INFO("{}", CI_MESSAGE);
#endif
  LOG_INFO("UE3-SDK-GEN started at {}", absl::FormatTime(start_time));

  try {
    GenerateSDK();
  } catch (std::exception& ex) {
    LOG_CRITICAL("Uncaught error during SDK generation: {}", ex.what());
    DebugBreak();
  }

  const auto end_time = absl::Now();
  const auto duration = end_time - start_time;
  LOG_INFO("Generator is complete in {}", absl::FormatDuration(duration));

  return EXIT_SUCCESS;
}
#endif
