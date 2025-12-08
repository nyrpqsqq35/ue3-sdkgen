//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#include "sdk.h"

namespace SDKGEN_NS::internal {

void CallWithExplicitObject(UFunction* fn, void* obj, void* in_out_params) {
  static_cast<UObject*>(obj)->ProcessEvent(fn, in_out_params, nullptr);
}
void CallWithExplicitObjectStripNative(UFunction* fn, void* obj, void* in_out_params) {
  fn->function_flags &= ~0x00000400;
  static_cast<UObject*>(obj)->ProcessEvent(fn, in_out_params, nullptr);
  fn->function_flags |= ~0x00000400;
}

}  // namespace SDKGEN_NS::internal
