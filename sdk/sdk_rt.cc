//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#include "sdk.h"
#include "sdk_config.h"

#include <format>
#include <fstream>
#include <thread>

namespace {
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
}  // namespace

namespace SDKGEN_NS {

Rt Rt::instance_;
TArray<UObject*>* GObjects;
TArray<FNameEntry*>* GNames;

uintptr_t OffsetConfig::Offset::operator()() const { return GetImageBaseAddress() + offset; }
uintptr_t OffsetConfig::Pattern::operator()() const {
  throw std::runtime_error("not implemented yet");
}

void Rt::Initialize() {
  for (const auto& [name, data, ptr, value] : kOffsetConfigs) {
    value = std::visit([](auto&& a) -> uintptr_t { return a(); }, data);
    if (value == 0) {
      throw std::runtime_error(std::format("failed to resolve {}", name));
    }
    if (ptr) {
      *static_cast<uintptr_t*>(ptr) = value;
    }
    // LOG_INFO("{} resolved to {:#016x}", name, value);
  }

  GObjects = reinterpret_cast<TArray<UObject*>*>(kObjectsOffset.value);
  GNames = reinterpret_cast<TArray<FNameEntry*>*>(kNamesOffset.value);

  for (auto* obj : *GObjects) {
    if (!obj) continue;
    // ->IsA only works here because the ::StaticClass on UFunction and UState use FindClassOld
    if (obj->IsA<UFunction>()) {
      const auto name_hash = SDKGEN_NS::StringHash<>::Calculate(obj->GetFullName());
      function_map_.insert({name_hash, static_cast<UFunction*>(obj)});
    } else if (obj->IsA<UClass>()) {
      const auto name_hash = SDKGEN_NS::StringHash<>::Calculate(obj->GetFullName());
      class_map_.insert({name_hash, static_cast<UClass*>(obj)});
    }
  }
}
UClass* Rt::FindClass(const HashType hash) {
  if (!class_map_.contains(hash)) {
    return nullptr;
  }
  return class_map_[hash];
}
UFunction* Rt::FindFunction(const HashType hash) {
  if (!function_map_.contains(hash)) {
    return nullptr;
  }
  return function_map_[hash];
}

decltype(Rt::ProcessEvent_Hook)* Rt::o_ProcessEvent;
void Rt::ProcessEvent_Hook(UObject* self, UFunction* fn, void* params) {
  if (!instance_.hooks_.contains(fn->object_internal_integer)) {
    return o_ProcessEvent(self, fn, params);
  }

  FunctionHook::Context ctx;
  const auto& fn_hooks = instance_.hooks_[fn->object_internal_integer];

  for (const auto& [type, function_handle] : fn_hooks) {
    switch (type) {
      case FunctionHook::kBlock:
        return;
      case FunctionHook::kPre:
        instance_.GetFunctionByHandle(function_handle)(params, self, fn, ctx);
        break;
      default:
        break;
    }
  }

  o_ProcessEvent(self, fn, params);

  for (const auto& [type, function_handle] : fn_hooks) {
    if (type == FunctionHook::kPost) {
      instance_.GetFunctionByHandle(function_handle)(params, self, fn, ctx);
    }
  }
}

int32_t Rt::GetObjInternalInteger(const UFunction* fn) noexcept {
  return fn->object_internal_integer;
}

Rt& rt() { return Rt::instance_; }

}  // namespace SDKGEN_NS
