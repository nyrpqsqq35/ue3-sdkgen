//
// Created on 03-Dec-25.
// Copyright (c) 2025. All rights reserved.
//

#include "bridge.h"
#include "util.h"
#include "config.h"
#include "processor_classes.h"
#include "processor_consts.h"
#include "processor_enums.h"
#include "processor_structs.h"
#include "unreal.h"

namespace {

using namespace bridge;
uintptr_t base_address = 0;

void FindAndDumpGNames(const util::Mapping& mapping) {
  const Pointer<int> v0(base_address + config::kGNamesOffset);
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
  const Pointer<int> v4(base_address + config::kGObjectsOffset);
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

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
  // Perform actions based on the reason for calling.
  switch (reason) {
    case DLL_PROCESS_ATTACH:
      std::thread([&]() {
        spdlog::set_level(spdlog::level::trace);
        MboxFmt("UE3-SDK-GEN started");
        const auto start_time = absl::Now();

        try {
          GenerateSDK();
        } catch (std::exception& ex) {
          MboxFmtErr("Uncaught error during SDK generation: {}", ex.what());
        }

        const auto end_time = absl::Now();
        const auto duration = end_time - start_time;
        MboxFmt("Generator is complete in {}", absl::FormatDuration(duration));
      }).detach();
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
