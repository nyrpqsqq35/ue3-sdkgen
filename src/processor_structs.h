#pragma once

#include <absl/container/flat_hash_set.h>

#include <format>
#include <functional>

#include "bridge.h"
#include "processor.h"
#include "unreal.h"

namespace processor {
namespace structs {

// LLVM 18 requires this to be declared before StandaloneProcessStruct
// Not sure why it was working on LLVM <= 17
void ProcessStruct(bridge::Pointer<UObject> obj);

namespace {
inline absl::flat_hash_map<uintptr_t, std::string> duplicate_member_names;
}
inline std::string GetPropertyNameForFunctionGen(const bridge::Pointer<UProperty>& prop) {
  if (duplicate_member_names.contains(prop.get())) {
    return duplicate_member_names[prop.get()];
  }
  return CreateValidName(prop->name.ToString());
}

template <bool process_children, bool verifier = false>
std::string StandaloneProcessStruct(
    bridge::Pointer<UObject> obj, std::stringstream& file,
    std::function<void(bridge::Pointer<UProperty>)> callback_func = nullptr) {
  bridge::Pointer<UStruct> ustruct(obj.get());

  auto package_obj = obj->GetPackageObject();
  auto& pkg = GetPackage(package_obj);

  std::vector<bridge::Pointer<UProperty>> child_properties;

  bridge::Pointer<UProperty> last_valid_child_for_verifier(0);
  std::string last_valid_child_for_verifier_member_name;

  bridge::Pointer<UProperty> child(ustruct->children.get());

  while (child.IsValid()) {
    if constexpr (process_children) {
      if (child->IsA("Class Core.StructProperty")) {
        bridge::Pointer<UStructProperty> up(child.get());
        auto struct_ = up->struct_;
        if (struct_->GetPackageObject() == package_obj) {
          if (!pkg.ProcessedStruct(struct_)) {
            ProcessStruct(bridge::Pointer<UObject>(struct_.get()));
          }
        }
      }
    }

    if (child->IsA("Class Core.Property")) {
      child_properties.emplace_back(child);
    } else {
      // we call the callback function here so the callback sees the Function properties
      if (callback_func) callback_func(child);
    }

    // Progress to next child
    child.Set(child->next.get());
  }

  std::sort(child_properties.begin(), child_properties.end(),
            [](bridge::Pointer<UProperty>& a, bridge::Pointer<UProperty>& b) {
              auto bleh = a->offset < b->offset;
              if (a->IsA("Class Core.BoolProperty") && b->IsA("Class Core.BoolProperty")) {
                bridge::Pointer<UBoolProperty> abp(a.get());
                bridge::Pointer<UBoolProperty> bbp(b.get());
                if (a->offset == b->offset) {
                  return abp->bitmask < bbp->bitmask;
                }
              }
              return bleh;
            });

  absl::flat_hash_set<std::string> used_names;
  int member_index = 0;
  bridge::Pointer<UProperty> last_child(0);

  if (child_properties.size() > 0) {
    auto first_child = *child_properties.begin();

    if (first_child.IsValid()) {
      if (ustruct->superfield.IsValid()) {
        bridge::Pointer<UStruct> superfield(ustruct->superfield.get());
        if (superfield->property_size < first_child->offset) {
          file << "SDKGEN_PADFIELD(" << std::dec << first_child->offset - superfield->property_size
               << "); // #1\n";
        }
      }
    }
  } else {
    if (ustruct->superfield.IsValid()) {
      bridge::Pointer<UStruct> superfield(ustruct->superfield.get());
      if (superfield->property_size < ustruct->property_size) {
        file << "SDKGEN_PADFIELD(" << std::dec << ustruct->property_size - superfield->property_size
             << "); // #2\n";
      }
    }
  }

  bool bitfield_union_open = false;
  for (auto& child : child_properties) {
    bool child_is_a_boolproperty = child->IsA("Class Core.BoolProperty");

    if (last_child.IsValid()) {
      if (bitfield_union_open && child->offset != last_child->offset) {
        file << "};\n";
        file << "uint32_t SDKGEN_CONCAT2(bitfield_, __COUNTER__);\n";
        file << "};\n";
        bitfield_union_open = false;
      }

      if (child->offset != last_child->offset) {
        auto last_child_size = GetPropertySize(last_child);
        auto expected_offset = last_child->offset + last_child_size;
        if (expected_offset < child->offset) {
          file << "SDKGEN_PADFIELD(" << std::dec << child->offset - expected_offset << "); // #3\n";
        } else if (expected_offset > child->offset) {
          LOG_WARN("offset invalid !!");
          file << "/* offset invalid !! */";
        }
      }
    }

    if (child_is_a_boolproperty) {
      bool should_open_union = false;
      if (last_child.IsValid()) {
        if (child->offset != last_child->offset) {
          should_open_union = true;
        }
      } else {
        should_open_union = true;
      }

      if (should_open_union) {
        bitfield_union_open = true;
        file << "union {\n";
        file << "struct {\n";
      }
    }
    auto child_property_size = GetPropertySize(child);

    if (child_is_a_boolproperty) {
      const bridge::Pointer<UBoolProperty> bp(child.get());
      uint32_t last_mask = 1 << 0;
      const uint32_t this_mask = bp->bitmask;

      if (last_child.IsValid() && last_child->IsA("Class Core.BoolProperty")) {
        const bridge::Pointer<UBoolProperty> last_bp(last_child.get());

        if (last_bp->offset == bp->offset) {
          last_mask = last_bp->bitmask << 1;
        }
      }

      for (uint32_t bit = last_mask; bit < this_mask; bit <<= 1) {
        file << GetPropertyCType(child) << " _pad_" << std::hex << bit << " : 1;\n";
      }
    }

    file << GetPropertyCType(child) << " ";

    auto struct_member_name = CreateValidName(child->name.ToString());

    file << struct_member_name;
    if (used_names.contains(struct_member_name)) {
      struct_member_name += std::to_string(member_index);
      file << std::dec << member_index;
      duplicate_member_names.insert({child.get(), struct_member_name});
    } else {
      used_names.insert(struct_member_name);
    }

    if (child->array_dim > 1) {
      file << "[" << std::dec << std::setw(0) << child->array_dim << "]";
    }
    if (child->IsA("Class Core.MapProperty")) {
      file << "[" << std::dec << std::setw(0) << child_property_size << "]";
    }

    if (child->IsA("Class Core.BoolProperty")) {
      // Bitfield
      file << " : 1";
    } else if constexpr (verifier) {
      last_valid_child_for_verifier.Set(child.get());
      last_valid_child_for_verifier_member_name = struct_member_name;
    }

    file << ";";

    file << " // " << std::hex << std::showbase << std::setfill('0') << child->offset;
    file << " size " << child_property_size;

    if (child->IsA("Class Core.BoolProperty")) {
      bridge::Pointer<UBoolProperty> bp(child.get());
      file << " " << "[" << std::hex << std::showbase << bp->bitmask << "]";
    }

    file << "\n";

    if (child->IsA("Class Core.StructProperty")) {
      bridge::Pointer<UStructProperty> usp(child.get());
      auto array_dim = child->array_dim == 0 ? 1 : child->array_dim;
      auto prop_size = usp->struct_->property_size * array_dim;
      if (prop_size < child_property_size) {
        file << "SDKGEN_PADFIELD(" << std::dec << child_property_size - prop_size << "); // #4\n";
      }
    }

    // cos the function gen needs the member name cached if its a duplicate
    if (callback_func) callback_func(child);

    last_child.Set(child.get());
    ++member_index;
  }

  if (bitfield_union_open) {
    file << "};\n";
    file << "uint32_t SDKGEN_CONCAT2(bitfield_, __COUNTER__);\n";
    file << "};\n";
    bitfield_union_open = false;
  }

  if (last_child.IsValid()) {
    auto expected_size = bridge::Pointer<UStruct>(obj.get())->property_size;
    auto actual_size = last_child->offset + GetPropertySize(last_child);
    if (actual_size < expected_size) {
      file << "SDKGEN_PADFIELD(" << std::dec << expected_size - actual_size << "); // #5\n";
    } else if (actual_size > expected_size) {
      LOG_WARN("invalid struct !!");
      file << "/* invalid struct !! */\n";
    }
  }

  if constexpr (verifier) {
    if (last_valid_child_for_verifier.IsValid()) {
      if (obj->GetFullName() != "Class IpDrv.HTTPDownload" /* doesn't count :^) */) {
        GetPropertySize(last_valid_child_for_verifier);
        processor::VerifierStatements.push_back(std::format(
            "static_assert( offsetof({}, {}) == {} );", CreateIdentifierName(obj),
            last_valid_child_for_verifier_member_name, last_valid_child_for_verifier->offset));
      }
    }
  }

  return file.str();
}

}  // namespace structs
}  // namespace processor
