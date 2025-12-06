//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once
#include "sdk_hash.h"

// due to EBO this will optimize out to zero length, not affecting the size of our classes/structs.
template <typename Params, SDKGEN_NS::StringHash<>::value_type NameHash, bool IsNative>
class CallableUFunction {
 public:
  void Call(Params& in_out_params) const {
    return CallWithExplicitObject((void*)this, &in_out_params);
  }
  static void CallWithExplicitObject(void* obj, Params* in_out_params) {
    static UFunction* fn = UFunction::FindFunction(NameHash);
    if constexpr (IsNative) {
      fn->function_flags &= ~0x00000400;
    }
    static_cast<UObject*>(obj)->ProcessEvent(fn, &in_out_params, nullptr);
    if constexpr (IsNative) {
      fn->function_flags |= ~0x00000400;
    }
  }
};

template <typename Params, SDKGEN_NS::StringHash<>::value_type NameHash, bool IsNative,
          typename Parent>
class CallableUFunctionStatic : public CallableUFunction<Params, NameHash, IsNative> {
 public:
  static void Call(Params& in_out_params) {
    return CallableUFunction<Params, NameHash, IsNative>::CallWithExplicitObject(
        Parent::StaticClass(), &in_out_params);
  }
};
