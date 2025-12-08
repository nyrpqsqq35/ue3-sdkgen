//
// Copyright (c) 2025 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once
#include "sdk_hash.h"

#include <algorithm>
#include <functional>
#include <unordered_map>

namespace SDKGEN_NS {

struct FunctionHook {
  enum HookType { kUnspecified = 0, kPre, kPost, kBlock };
  struct Context {};

  using FnType = std::function<void(void* params, UObject* obj, UFunction* fn, Context& ctx)>;
  template <typename T>
  using FnTypeTemplated = std::function<void(T* params, UObject* obj, UFunction* fn, Context& ctx)>;
  using HandleType = int32_t;

  static constexpr HandleType kInvalidHandle = -1;
  static constexpr HandleType kMaxHandles =
      std::numeric_limits<FunctionHook::HandleType>::max() - 1;

  HookType type{kUnspecified};
  HandleType function_handle{kInvalidHandle};
};

class Rt {
  using HashType = SDKGEN_NS::StringHash<>;

 public:
  // Throws std::runtime_error if there's a problem
  void Initialize();

  UClass* FindClass(HashType hash);
  UFunction* FindFunction(HashType hash);

  // hooking (NOT thread safe!!!!)

  void AddBlock(UFunction* ufn) {
    hooks_[GetObjInternalInteger(ufn)].emplace_back(FunctionHook::kBlock,
                                                    FunctionHook::kInvalidHandle);
  }
  void RemoveBlock(UFunction* ufn) {
    if (const auto obj_internal_integer = GetObjInternalInteger(ufn);
        hooks_.contains(obj_internal_integer)) {
      auto& fn_hooks = hooks_[obj_internal_integer];
      std::erase_if(fn_hooks, [](FunctionHook& hk) { return hk.type == FunctionHook::kBlock; });
    }
  }

  template <FunctionHook::HookType Type>
  FunctionHook::HandleType AddHook(UFunction* ufn, FunctionHook::FnType hook_fn) {
    if (hook_functions_.size() >= FunctionHook::kMaxHandles) {
      throw std::range_error("Too many functions");
    }

    hook_functions_.emplace_back(hook_fn);
    const auto fn_handle = hook_functions_.size() - 1;

    hooks_[GetObjInternalInteger(ufn)].emplace_back(
        Type, static_cast<FunctionHook::HandleType>(fn_handle));

    return fn_handle;
  }
  void RemoveHook(UFunction* ufn, FunctionHook::HandleType handle) {
    if (const auto obj_internal_integer = GetObjInternalInteger(ufn);
        hooks_.contains(obj_internal_integer)) {
      auto& fn_hooks = hooks_[obj_internal_integer];
      std::erase_if(fn_hooks, [handle](FunctionHook& hk) { return hk.function_handle == handle; });
    }

    DeleteFunctionHandle(handle);
  }
  void RemoveHooks(UFunction* ufn) {
    if (const auto obj_internal_integer = GetObjInternalInteger(ufn);
        hooks_.contains(obj_internal_integer)) {
      for (const auto& fn_hooks = hooks_[obj_internal_integer];
           const auto& [type, function_handle] : fn_hooks) {
        if (function_handle != FunctionHook::kInvalidHandle) {
          DeleteFunctionHandle(function_handle);
        }
      }
      hooks_.erase(obj_internal_integer);
    }
  }

  void DeleteFunctionHandle(FunctionHook::HandleType handle) {
    if (handle >= hook_functions_.size() || handle == FunctionHook::kInvalidHandle) {
      throw std::range_error("Invalid function handle");
    }

    // Can't reclaim (for now)
    hook_functions_[handle] = nullptr;
  }
  const FunctionHook::FnType& GetFunctionByHandle(FunctionHook::HandleType handle) {
    return hook_functions_[handle];
  }

 public:
  static void ProcessEvent_Hook(UObject* self, UFunction* fn, void* params);
  static decltype(ProcessEvent_Hook)* o_ProcessEvent;

 private:
  static int32_t GetObjInternalInteger(const UFunction* fn) noexcept;

 public:
  std::unordered_map<HashType, UClass*> class_map_;
  std::unordered_map<HashType, UFunction*> function_map_;

  std::unordered_map<int32_t /* obj_internal_integer */, std::vector<FunctionHook>> hooks_;
  std::vector<FunctionHook::FnType> hook_functions_;

 private:
  static Rt instance_;
  friend Rt& rt();
};

Rt& rt();

}  // namespace SDKGEN_NS
