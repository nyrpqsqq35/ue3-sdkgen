//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once
#include "sdk_hash.h"
#include "sdk_rt.h"

namespace SDKGEN_NS::/* not so */ internal {
void CallWithExplicitObject(UFunction* fn, void* obj, void* in_out_params);
void CallWithExplicitObjectStripNative(UFunction* fn, void* obj, void* in_out_params);
}  // namespace SDKGEN_NS::internal

#define FIND_FUNCTION(NameHash) SDKGEN_NS::rt().FindFunction(NameHash)

// due to EBO this will optimize out to zero length, not affecting the size of our classes/structs.
template <typename Params, SDKGEN_NS::StringHash<>::value_type NameHash, bool IsNative>
class CallableUFunction {
 public:
  void Call(Params& in_out_params) const {
    // So our generated classes include callable UFunctions
    // They are generated as [[no_unique_address]] struct ... : CallableUFunction<...> { ... }
    // There is no guaranteed placement for the struct inside its parent with [[no_unique_address]]
    // We are relying on the idea that `this` will always be +0 from its parent
    // This is not declared in the standard, so this may break.
    // If this becomes a problem maybe we could add new template parameters to calculate the offset.
    return CallWithExplicitObject((void*)this, &in_out_params);
  }
  static void CallWithExplicitObject(void* obj, Params* in_out_params) {
    static UFunction* fn = FIND_FUNCTION(NameHash);
    // UObject/UFunction may not fully exist at the time this template class is used
    if constexpr (IsNative) {
      return SDKGEN_NS::internal::CallWithExplicitObjectStripNative(fn, obj, in_out_params);
    } else {
      return SDKGEN_NS::internal::CallWithExplicitObject(fn, obj, in_out_params);
    }
  }

  static void Block() { return SDKGEN_NS::rt().AddBlock(FIND_FUNCTION(NameHash)); }
  static void Unblock() { return SDKGEN_NS::rt().RemoveBlock(FIND_FUNCTION(NameHash)); }
  static SDKGEN_NS::FunctionHook::HandleType HookPre(
      SDKGEN_NS::FunctionHook::FnTypeTemplated<Params> hook_fn) {
    return SDKGEN_NS::rt().AddHook<SDKGEN_NS::FunctionHook::kPre>(
        FIND_FUNCTION(NameHash), [hook_fn](void* params, auto* obj, auto* fn, auto& ctx) {
          return hook_fn(static_cast<Params*>(params), obj, fn, ctx);
        });
  }
  static SDKGEN_NS::FunctionHook::HandleType HookPost(
      SDKGEN_NS::FunctionHook::FnTypeTemplated<Params> hook_fn) {
    return SDKGEN_NS::rt().AddHook<SDKGEN_NS::FunctionHook::kPost>(
        FIND_FUNCTION(NameHash), [hook_fn](void* params, auto* obj, auto* fn, auto& ctx) {
          return hook_fn(static_cast<Params*>(params), obj, fn, ctx);
        });
  }
  static void RemoveHook(SDKGEN_NS::FunctionHook::HandleType handle) {
    return SDKGEN_NS::rt().RemoveHook(FIND_FUNCTION(NameHash), handle);
  }
  static void RemoveAllHooks() { SDKGEN_NS::rt().RemoveHooks(FIND_FUNCTION(NameHash)); }
};

#undef FIND_FUNCTION

template <typename Params, SDKGEN_NS::StringHash<>::value_type NameHash, bool IsNative,
          typename Parent>
class CallableUFunctionStatic : public CallableUFunction<Params, NameHash, IsNative> {
 public:
  static void Call(Params& in_out_params) {
    return CallableUFunction<Params, NameHash, IsNative>::CallWithExplicitObject(
        Parent::StaticClass(), &in_out_params);
  }
};
