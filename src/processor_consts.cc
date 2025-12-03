#include "processor_consts.h"

#include "processor.h"

namespace processor {
namespace consts {
void ProcessConst(bridge::Pointer<UObject>& obj) {
  bridge::Pointer<UConst> uconst(obj.get());

  // Value is empty
  if (uconst->value.count() == 0) return;

  auto& pkg = GetPackage(uconst->GetPackageObject());
  pkg.generated_classes.emplace_back(uconst.get(), "#define CONST_" + CreateIdentifierName(uconst) +
                                                       " " + uconst->value.ToString() + "\n");
}
}  // namespace consts
}  // namespace processor
