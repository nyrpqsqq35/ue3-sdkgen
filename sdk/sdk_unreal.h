//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once
#include "sdk_unreal_array.h"
// ReSharper disable CppClassNeedsConstructorBecauseOfUninitializedMember

extern TArray<class UObject*>* GObjects;
extern TArray<struct FNameEntry*>* GNames;

struct FNameEntry {
  uint64_t flags;
  int32_t index;
  SDKGEN_PADFIELD(0xc);
  game_char name[0x400];
  [[nodiscard]] std::string ToString() const;
};
struct FName {
  int32_t entry_id;
  int32_t instance_number;

  [[nodiscard]] std::string ToString() const;
};

class FString : public TArray<game_char> {
 public:
  [[nodiscard]] std::string ToString() const;
};

class UObject {
 public:
  FPointer vftable;
  FPointer hash_next;
  uint64_t object_flags;
  FPointer hash_outer_next;
  FPointer state_frame;
  UObject* linker;
  FPointer linker_index;
  int32_t object_internal_integer;
  int32_t net_index;
  UObject* outer;
  FName name;
  UClass* class_;
  UObject* object_archetype;

 public:
  static UClass* StaticClass() {
    static UClass* clazz = UObject::FindClass("Class Core.Object");
    return clazz;
  }
  [[nodiscard]] std::string GetFullName() const;
  [[nodiscard]] std::string GetPackageName() const;
  [[nodiscard]] bool IsA(const UClass* uclass) const;
  [[nodiscard]] bool IsA(int32_t obj_internal_integer) const;
  void ProcessEvent(class UFunction* uFunction, void* uParams, void* uResult);

  template <typename T>
  [[nodiscard]] bool IsA() const {
    if (std::is_base_of_v<UObject, T>) {
      return IsA(T::StaticClass());
    }
    return false;
  }

  static TArray<UObject*>& GObjects();
  static TArray<FNameEntry*>& GNames();
  static UClass* FindClass(std::string_view class_full_name);
  static UClass* FindClass(SDKGEN_NS::StringHash<> hash);
  template <typename T>
  static T* FindObject(const std::string& object_full_name) {
    for (UObject* obj : GObjects()) {
      if (obj && obj->IsA<T>()) {
        if (obj->GetFullName() == object_full_name) {
          return static_cast<T*>(obj);
        }
      }
    }
    return nullptr;
  }
};
static_assert(sizeof(UObject) == 96);

class UField : public UObject {
 public:
  UField* next;
  UField* superfield;
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
};
static_assert(sizeof(UProperty) == 200);

class UStruct : public UField {
 public:
  SDKGEN_PADFIELD(0x10);
  UField* superfield;
  UField* children;
  uint32_t property_size;
  SDKGEN_PADFIELD(0x9c);
};
static_assert(sizeof(UStruct) == 304);

struct FScriptDelegate {
  class UObject* object;
  SDKGEN_PADFIELD(0x10);
};

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

 public:
  static UClass* StaticClass() {
    static UClass* clazz = UObject::FindClass("Class Core.Object");
    return clazz;
  }
  static UFunction* FindFunction(std::string_view function_full_name);
  static UFunction* FindFunction(SDKGEN_NS::StringHash<> hash);
};

static_assert(sizeof(UFunction) == 352);

class UState : public UStruct {
 public:
  SDKGEN_PADFIELD(0x60);
};
static_assert(sizeof(UState) == 400);

class UClass : public UState {
 public:
  static UClass* StaticClass() {
    static UClass* clazz = UObject::FindClass("Class Core.Class");
    return clazz;
  }
  SDKGEN_PADFIELD(0x228);
};
static_assert(sizeof(UClass) == 952);
