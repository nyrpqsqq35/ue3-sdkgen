//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#include "processor_structs.h"

#include <sstream>

#include "processor.h"

namespace processor {
namespace structs {

void ProcessStruct(bridge::Pointer<UObject> obj) {
  bridge::Pointer<UStruct> ustruct(obj.get());

  if (obj->GetFullName() == "ScriptStruct Core.Object.Pointer") return;

  auto package_obj = obj->GetPackageObject();
  auto& pkg = GetPackage(package_obj);

  if (pkg.ProcessedStruct(ustruct)) return;

  std::stringstream file;

  file << "// Property size: ";
  file << std::hex << std::showbase << ustruct->property_size;
  file << std::dec << " (" << ustruct->property_size << ")\n";

  const auto our_name = CreateIdentifierName(ustruct);
  Package::JsonObject json{obj, our_name, ustruct->property_size};

  file << "struct " << our_name;

  if (ustruct->superfield.IsValid()) {
    const auto their_name = CreateIdentifierName(ustruct->superfield);
    file << " : " << their_name;
    file << " /* " << ustruct->superfield->GetFullName() << " */";
    json.inheritance.emplace(their_name, ustruct->superfield->GetPackageObject()->name.ToString());

    if (ustruct->superfield->GetPackageObject() == package_obj) {
      if (!pkg.ProcessedStruct(ustruct->superfield)) {
        ProcessStruct(bridge::Pointer<UObject>(ustruct->superfield.get()));
      }
    }
  }

  file << " {\n";

  StandaloneProcessStruct<true, true>(obj, file, nullptr, [&json](bridge::Pointer<UProperty> prop) {
    json.props.emplace_back(prop->name.ToString(), prop->property_flags, GetPropertyCType(prop),
                            prop->array_dim, prop->offset);
  });

  file << "};" << std::endl;

  // file << "static_assert(sizeof(" << our_name << ") == " << std::dec << e->property_size << ");"
  //      << std::endl;

  pkg.generated_structs.push_back({ustruct.get(), file.str()});
  pkg.json_structs.emplace_back(json);
}
}  // namespace structs
}  // namespace processor
