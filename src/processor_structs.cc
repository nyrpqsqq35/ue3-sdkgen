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

  file << "struct " << our_name;

  if (ustruct->superfield.IsValid()) {
    file << " : " << CreateIdentifierName(ustruct->superfield);
    file << " /* " << ustruct->superfield->GetFullName() << " */";

    if (ustruct->superfield->GetPackageObject() == package_obj) {
      if (!pkg.ProcessedStruct(ustruct->superfield)) {
        ProcessStruct(bridge::Pointer<UObject>(ustruct->superfield.get()));
      }
    }
  }

  file << " {\n";

  StandaloneProcessStruct<true, true>(obj, file);

  file << "};" << std::endl;

  // file << "static_assert(sizeof(" << our_name << ") == " << std::dec << e->property_size << ");"
  //      << std::endl;

  pkg.generated_structs.push_back({ustruct.get(), file.str()});
}
}  // namespace structs
}  // namespace processor
