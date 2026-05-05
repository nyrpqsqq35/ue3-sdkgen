//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#include "processor_consts.h"

#include "processor.h"

namespace processor {
namespace consts {
void ProcessConst(bridge::Pointer<UObject>& obj) {
  bridge::Pointer<UConst> uconst(obj.get());

  // Value is empty
  if (uconst->value.count() == 0) return;

  auto& pkg = GetPackage(uconst->GetPackageObject());
  const auto our_name = CreateIdentifierName(uconst);
  pkg.generated_classes.emplace_back(
      uconst.get(), "#define CONST_" + our_name + " " + uconst->value.ToString() + "\n");
  pkg.json_consts.emplace_back(our_name, uconst->value.ToString());
}
}  // namespace consts
}  // namespace processor
