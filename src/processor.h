//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <concepts>
#include <filesystem>
#include <fstream>

#include "bridge.h"
#include "unreal.h"

#include <absl/container/node_hash_map.h>

namespace processor {
inline std::filesystem::path sdk_path = std::filesystem::current_path() / ".." / ".." / "sdk";

typedef struct Package {
 public:
  struct GeneratedObject {
    bridge::Pointer<UObject> ptr;
    std::string generated_cpp;
    std::string generated_header;  // optional
    std::string generated_source;  // optional
  };

  std::list<GeneratedObject> generated_structs;
  std::list<GeneratedObject> generated_classes;

  template <std::derived_from<UObject> T>
  inline bool ProcessedStruct(bridge::Pointer<T> struct_to_check) const {
    auto begin = generated_structs.cbegin();
    auto end = generated_structs.cend();
    return std::find_if(begin, end, [struct_to_check](const auto& item) {
             return item.ptr == struct_to_check.get();
           }) != end;
  }
  template <std::derived_from<UObject> T>
  inline bool ProcessedClass(bridge::Pointer<T> class_to_check) const {
    auto begin = generated_classes.cbegin();
    auto end = generated_classes.cend();
    return std::find_if(begin, end, [class_to_check](const auto& item) {
             return item.ptr == class_to_check.get();
           }) != end;
  }
} Package;
Package& GetPackage(const bridge::Pointer<UObject>& package);
void CreatePackage(bridge::Pointer<UObject>& package);
bool HasPackageBeenCreated(bridge::Pointer<UObject>& package);
const absl::node_hash_map<bridge::Pointer<UObject>, Package>& GetAllPackages();

inline std::string CreateValidName(std::string name) {
  for (char& c : name) {
    if (c >= '!' && c <= '/') {
      c = '_';
    } else if (c >= ':' && c <= '@') {
      c = '_';
    } else if (c >= '[' && c <= '`') {
      c = '_';
    } else if (c == '{' || c == '}' || c == '|') {
      c = '_';
    }
  }
  return name;
}

inline absl::flat_hash_map<uintptr_t, std::string> ObjectToIdentifierMap;
inline absl::flat_hash_set<std::string> UsedIdentifiers;
inline absl::flat_hash_map<std::string, int> IdentifierIndexCount;
inline std::vector<std::string> VerifierStatements;

inline bool HasIdentifierBeenUsed(const std::string& identifier) {
  return UsedIdentifiers.contains(identifier);
}

constexpr const char kStructPrefix = 'F';
constexpr const char kClassPrefix = 'U';
constexpr const char kActorPrefix = 'A';

template <std::derived_from<UObject> T>
std::string CreateIdentifierName(bridge::Pointer<T>& object) {
  if (ObjectToIdentifierMap.contains(object.get())) {
    return ObjectToIdentifierMap[object.get()];
  }

  auto name = CreateValidName(object->name.ToString());
  if (UsedIdentifiers.contains(name)) {
    auto& identifier_count = IdentifierIndexCount[name];
    name += std::to_string(++identifier_count);
  } else {
    IdentifierIndexCount.insert({name, 1});
    UsedIdentifiers.insert(name);
  }

  auto& start_char = *name.begin();

  // The order of branches is important since inheritance goes  actor > class > struct
  if (object->IsA("Class Core.Function")) {
  } else if (object->IsA("Class Engine.Actor")) {
    name = kActorPrefix + name;
  } else if (object->IsA("Class Core.Class")) {
    name = kClassPrefix + name;
  } else if (object->IsA("Class Core.Struct")) {
    name = kStructPrefix + name;
  }

  ObjectToIdentifierMap.insert({object.get(), name});
  return name;
}

constexpr const auto kStructTypenamePrefix = "struct ";
constexpr const auto kClassTypenamePrefix = "class ";

constexpr const auto kStrType = "FString";
constexpr const auto kQwordType = "uint64_t";
constexpr const auto kNameType = "FName";
const auto kMapType = "TMap"s;
constexpr const auto kIntType = "int32_t";
constexpr const auto kFloatType = "float";
constexpr const auto kByteType = "uint8_t";
constexpr const auto kDelegateType = "struct FScriptDelegate";
constexpr const auto kBoolType = "uint32_t";
const auto kArrayType = "TArray"s;

// template<std::derived_from<UProperty> T>
inline const std::string GetPropertyCType(const bridge::Pointer<UProperty>& property) {
  if (property->IsA("Class Core.StructProperty")) {
    bridge::Pointer<UStructProperty> up(property.get());
    return kStructTypenamePrefix + CreateIdentifierName(up->struct_);
  } else if (property->IsA("Class Core.StrProperty")) {
    return kStrType;
  } else if (property->IsA("Class Core.QWordProperty")) {
    return kQwordType;
  } else if (property->IsA("Class Core.ObjectProperty")) {
    bridge::Pointer<UObjectProperty> up(property.get());
    return kClassTypenamePrefix + CreateIdentifierName(up->property_class) + "*";
  } else if (property->IsA("Class Core.ComponentProperty")) {
    // ?
  } else if (property->IsA("Class Core.ClassProperty")) {
    bridge::Pointer<UClassProperty> up(property.get());
    return kClassTypenamePrefix + CreateIdentifierName(up->meta_class) + "*";
  } else if (property->IsA("Class Core.NameProperty")) {
    return kNameType;
  } else if (property->IsA("Class Core.MapProperty")) {
    bridge::Pointer<UMapProperty> up(property.get());
    if (up->key.IsValid() && up->value.IsValid())
      return kMapType + "<" + GetPropertyCType(up->key) + "," + GetPropertyCType(up->value) + ">";
  } else if (property->IsA("Class Core.IntProperty")) {
    return kIntType;
  } else if (property->IsA("Class Core.InterfaceProperty")) {
    bridge::Pointer<UInterfaceProperty> up(property.get());
    return kClassTypenamePrefix + CreateIdentifierName(up->interface_class) + "*";
  } else if (property->IsA("Class Core.FloatProperty")) {
    return kFloatType;
  } else if (property->IsA("Class Core.DelegateProperty")) {
    return kDelegateType;
  } else if (property->IsA("Class Core.ByteProperty")) {
    return kByteType;
  } else if (property->IsA("Class Core.BoolProperty")) {
    return kBoolType;
  } else if (property->IsA("Class Core.ArrayProperty")) {
    bridge::Pointer<UArrayProperty> up(property.get());
    return kArrayType + "<" + GetPropertyCType(up->inner) + ">";
  }
  LOG_INFO("Unknown type {}", property->GetFullName());
  // DLOG(INFO) << "Unknown type " << property->GetFullName();
  return "/* unknown type */ uint8_t";
}

inline const size_t GetPropertySize(bridge::Pointer<UProperty>& property) {
  const auto size = [property]() -> size_t {
    if (property->IsA("Class Core.StructProperty")) {
      return property->element_size;
    } else if (property->IsA("Class Core.StrProperty")) {
      return sizeof(FString);
    } else if (property->IsA("Class Core.QWordProperty")) {
      return sizeof(uint64_t);
    } else if (property->IsA("Class Core.ObjectProperty")) {
      return sizeof(uintptr_t);
    } else if (property->IsA("Class Core.ComponentProperty")) {
      // ?
    } else if (property->IsA("Class Core.ClassProperty")) {
      return sizeof(uintptr_t);
    } else if (property->IsA("Class Core.NameProperty")) {
      return sizeof(FName);
    } else if (property->IsA("Class Core.MapProperty")) {
      return property->element_size;
    } else if (property->IsA("Class Core.IntProperty")) {
      return sizeof(int32_t);
    } else if (property->IsA("Class Core.InterfaceProperty")) {
      return sizeof(uintptr_t);
    } else if (property->IsA("Class Core.FloatProperty")) {
      return sizeof(float);
    } else if (property->IsA("Class Core.DelegateProperty")) {
      return property->element_size;
    } else if (property->IsA("Class Core.ByteProperty")) {
      return property->element_size;
    } else if (property->IsA("Class Core.BoolProperty")) {
      return property->element_size;
    } else if (property->IsA("Class Core.ArrayProperty")) {
      return sizeof(TArray<void>);
    }
    LOG_INFO("Unknown type {}", property->GetFullName());
    return -1;
  }();
  return size * property->array_dim;
}

// template <std::derived_from<UObject> T>
// std::string GetIdentifierName(bridge::Pointer<T>& object) {
//   if (ObjectToIdentifierMap.contains(object.get())) {
//     return ObjectToIdentifierMap[object.get()];
//   }

//   return "?";
// }

}  // namespace processor
