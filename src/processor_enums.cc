#include "processor_enums.h"

#include <absl/container/flat_hash_set.h>

#include "processor.h"

namespace processor {
namespace enums {

void ProcessEnum(bridge::Pointer<UObject>& obj) {
  bridge::Pointer<UEnum> uenum(obj.get());
  auto& pkg = GetPackage(uenum->GetPackageObject());
  std::stringstream file;

  file << "enum class " << CreateIdentifierName(obj) << " : uint8_t {\n";

  absl::flat_hash_set<std::string> used_names;
  auto names = uenum->names.Fetch();
  int name_index = 0;
  for (const auto& name : names) {
    auto enum_member_name = CreateValidName(name.ToString());

    // match behavior with other sdk gen
    if (enum_member_name.ends_with("_MAX"))
      enum_member_name.replace(enum_member_name.end() - 4, enum_member_name.end(), "_END");

    file << enum_member_name;

    if (used_names.contains(enum_member_name)) {
      // Dedupe enum member name by adding the index to the end of the member name
      file << name_index;
    } else {
      used_names.insert(enum_member_name);
    }

    file << ",\n";
    ++name_index;
  }
  file << "};\n" << std::endl;

  pkg.generated_classes.emplace_back(uenum.get(), file.str());
}

}  // namespace enums
}  // namespace processor
