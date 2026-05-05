//
// Created on 03-Dec-25.
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once

#include "bridge.h"
#include <absl/container/flat_hash_map.h>

#include <cstdint>

#define SDKGEN_CONCAT1(A_, B_) A_##B_
#define SDKGEN_CONCAT2(A_, B_) SDKGEN_CONCAT1(A_, B_)
#define SDKGEN_PADFIELD(Size_)                   \
 private:                                        \
  char SDKGEN_CONCAT2(pad_, __COUNTER__)[Size_]; \
                                                 \
 public:

using FPointer = uintptr_t;

using game_char = wchar_t;

#include "unreal_enums.h"

#pragma pack(push, 8)

struct FNameEntry {
  uint64_t Flags;
  int32_t Index;
  uint8_t UnknownDta[0xc];
  game_char Name[0x400];
};
struct FName {
  int32_t fname_entry_id;
  int32_t instance_number;

  [[nodiscard]] std::string ToString() const;
};

class UObject;
class UField;
class UStruct;
class UState;
class UClass;

template <typename T>
class TArray {
  using Self = TArray<T>;

 public:
  [[nodiscard]] void* data() const { return data_; }
  [[nodiscard]] int32_t count() const { return count_; }
  [[nodiscard]] int32_t max() const { return max_; }
  std::vector<T> Fetch() const {
    // data_ = *Pointer<void*>(base_ + offsetof(Self, data_));
    // count_ = *Pointer<int32_t>(base_ + offsetof(Self, count_));
    // max_ = *Pointer<int32_t>(base_ + offsetof(Self, max_));

    std::vector<T> vec(count());
    const auto size = sizeof(T) * count();
    // DLOG(INFO) << typeid(Self).name() << " is reading " << nSize << " bytes";
    ReadProcessMemory(bridge::process, data_, vec.data(), size, nullptr);
    return vec;
  }

 private:
  void* data_{};
  int32_t count_{};
  int32_t max_{};
};

class FString : public TArray<game_char> {
 public:
  std::string ToString() const;
};

class UObject {
 public:
  FPointer vftable;
  FPointer hash_next;
  EObjectFlags object_flags;
  FPointer hash_outer_next;
  FPointer state_frame;
  bridge::Pointer<UObject> linker;
  FPointer linker_index;
  int32_t object_internal_integer;
  int32_t net_index;
  bridge::Pointer<UObject> outer;
  FName name;
  bridge::Pointer<UClass> class_;
  bridge::Pointer<UObject> object_archetype;

 public:
  std::string GetFullName() const;
  std::string GetPackageName() const;
  bridge::Pointer<UObject> GetPackageObject() const;
  bool IsA(bridge::Pointer<UClass> uclass) const;
  bool IsA(const std::string& class_name) const;
};
static_assert(sizeof(UObject) == 96);

class UField : public UObject {
 public:
  bridge::Pointer<UField> next;
  bridge::Pointer<UField> superfield;
};
static_assert(sizeof(UField) == 112);

class UEnum : public UField {
 public:
  TArray<FName> names;
};
static_assert(sizeof(UEnum) == 128);
class UConst : public UField {
 public:
  FString value;
};
static_assert(sizeof(UConst) == 128);
class UProperty : public UField {
 public:
  unsigned long array_dim;
  unsigned long element_size;
  uint64_t property_flags;
  SDKGEN_PADFIELD(0x10);
  uint32_t property_size;
  SDKGEN_PADFIELD(0x4);
  unsigned long offset;
  SDKGEN_PADFIELD(0x2c);

 public:
  const std::string GetCType() const;
};
static_assert(sizeof(UProperty) == 200);

class UStruct : public UField {
 public:
  SDKGEN_PADFIELD(0x10);
  bridge::Pointer<UField> superfield;
  bridge::Pointer<UField> children;
  uint32_t property_size;
  SDKGEN_PADFIELD(0x9c);
};
static_assert(sizeof(UStruct) == 304);

class UFunction : public UStruct {
 public:
  uint64_t function_flags;
  uint16_t i_native;
  uint16_t rep_offset;
  FName friendly_name;
  uint8_t operator_precedence;
  uint8_t num_params;
  uint16_t params_size;
  unsigned long ret_val_offset;
  SDKGEN_PADFIELD(0xc);
  FPointer func;
};
static_assert(sizeof(UFunction) == 352);

class UScriptStruct : public UStruct {
 public:
  SDKGEN_PADFIELD(0x28);
};
static_assert(sizeof(UScriptStruct) == 344);

class UState : public UStruct {
 public:
  SDKGEN_PADFIELD(0x60);
};
static_assert(sizeof(UState) == 400);

class UClass : public UState {
 public:
  SDKGEN_PADFIELD(0x228);
};
static_assert(sizeof(UClass) == 952);

class UStructProperty : public UProperty {
 public:
  bridge::Pointer<UStruct> struct_;
};
static_assert(sizeof(UStructProperty) == 208);

class UStrProperty : public UProperty {};
static_assert(sizeof(UStrProperty) == 200);

class UQWordProperty : public UProperty {};
static_assert(sizeof(UQWordProperty) == 200);

class USQWordProperty : public UProperty {};
static_assert(sizeof(USQWordProperty) == 200);

class UObjectProperty : public UProperty {
 public:
  bridge::Pointer<UClass> property_class;
  SDKGEN_PADFIELD(0x8);
};
static_assert(sizeof(UObjectProperty) == 216);

class UComponentProperty : public UProperty {};
static_assert(sizeof(UComponentProperty) == 200);

class UClassProperty : public UProperty {
 public:
  bridge::Pointer<UClass> meta_class;
};
static_assert(sizeof(UClassProperty) == 208);

class UNameProperty : public UProperty {};
static_assert(sizeof(UNameProperty) == 200);

class UMapProperty : public UProperty {
 public:
  bridge::Pointer<UProperty> key;
  bridge::Pointer<UProperty> value;
};
static_assert(sizeof(UMapProperty) == 216);

class UIntProperty : public UProperty {};
static_assert(sizeof(UIntProperty) == 200);

class UInterfaceProperty : public UProperty {
 public:
  bridge::Pointer<UClass> interface_class;
  SDKGEN_PADFIELD(0x8);
};
static_assert(sizeof(UInterfaceProperty) == 216);

class UFloatProperty : public UProperty {};
static_assert(sizeof(UFloatProperty) == 200);

class UDelegateProperty : public UProperty {
 public:
  bridge::Pointer<UFunction> delegate_function;
  FName delegate_name;
};
static_assert(sizeof(UDelegateProperty) == 216);

class UByteProperty : public UProperty {
 public:
  bridge::Pointer<UEnum> enum_;
};
static_assert(sizeof(UByteProperty) == 208);

class UBoolProperty : public UProperty {
 public:
  uint64_t bitmask;
};
static_assert(sizeof(UBoolProperty) == 208);

class UArrayProperty : public UProperty {
 public:
  bridge::Pointer<UProperty> inner;
};
static_assert(sizeof(UArrayProperty) == 208);
#pragma pack(pop)

inline bridge::Pointer<TArray<bridge::Pointer<FNameEntry>>> GNames;
inline bridge::Pointer<TArray<bridge::Pointer<UObject>>> GObjects;
inline absl::flat_hash_map<int32_t, std::wstring> FNameIndexToString;
inline absl::flat_hash_map<std::string, bridge::Pointer<UClass>> StringToClass;
