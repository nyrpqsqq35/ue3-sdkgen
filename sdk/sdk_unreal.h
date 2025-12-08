//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once
#include "sdk_unreal_array.h"
#include "Core_macros.h"
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

// TODO: the null termination is kinda busted......
class FString : public TArray<game_char> {
 public:
  using traits_type = std::char_traits<value_type>;
  constexpr static size_type npos = -1;

 public:
  constexpr FString() {
    push_back(0);  // null terminate
  }
  constexpr FString(const std::basic_string_view<value_type, traits_type> wsv) { assign(wsv); }
  constexpr FString(const value_type* s) { assign(s); }
  constexpr FString(const value_type* s, size_type count) { assign(s, count); }
  [[nodiscard]] std::string ToString() const;

  constexpr FString& assign(const FString& str) { return *this = str; }
  constexpr FString& assign(FString&& str) noexcept { return *this = std::move(str); }
  constexpr FString& assign(size_type count, value_type ch) {
    clear();
    resize(count + 1, ch);
    (*this)[count] = 0;  // null terminate
    return *this;
  }
  constexpr FString& assign(const value_type* s, size_type count) {
    // 4
    clear();
    reserve(count + 1);
    insert(begin(), s, s + count);
    push_back(0);  // null terminate
    return *this;
  }
  constexpr FString& assign(const value_type* s) { return assign(s, traits_type::length(s)); }
  template <class SV>
  constexpr FString& assign(const SV& t) {
    std::basic_string_view<value_type, traits_type> sv = t;
    return assign(sv.data(), sv.size());
  }
  template <class SV>
  constexpr FString& assign(const SV& t, size_type pos, size_type count = npos) {
    std::basic_string_view<value_type, traits_type> sv = t;
    return assign(sv.substr(pos, count));
  }
  constexpr FString& assign(const FString& str, size_type pos, size_type count = npos) {
    return assign(std::basic_string_view<value_type, traits_type>(str).substr(pos, count));
  }
  template <class InputIt>
  constexpr FString& assign(InputIt first, InputIt last) {
    clear();
    while (first != last) {
      push_back(*first++);
    }
    push_back(0);  // null terminate
    return *this;
    // return assign(FString(first, last));
  }
  constexpr FString& assign(std::initializer_list<value_type> ilist) {
    return assign(ilist.begin(), ilist.size());
  }

  size_type size() const noexcept { return std::max(size_ - 1, 0); }
  size_type length() const noexcept { return size(); }

  bool operator==(const FString& other) const noexcept { return ToString() == other.ToString(); }
  std::strong_ordering operator<=>(const FString& other) const noexcept {
    return ToString() <=> other.ToString();
  }
  bool operator==(const std::string_view other) const noexcept { return ToString() == other; }
  std::strong_ordering operator<=>(const std::string_view& other) const noexcept {
    return ToString() <=> other;
  }
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
    if constexpr (std::is_base_of_v<UObject, T>) {
      return IsA(T::StaticClass());
    }
    return false;
  }

  static TArray<UObject*>& GObjects();
  static TArray<FNameEntry*>& GNames();
  static UClass* FindClassOld(std::string_view class_full_name);
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

  template <typename T>
  static T* FindInstance() {
    for (UObject* obj : GObjects()) {
      if (obj && obj->IsA<T>()) {
        if (obj->GetFullName().contains("Default__")) {
          continue;
        }
        return static_cast<T*>(obj);
      }
    }
    return nullptr;
  }

 public:
  Object_FUNCS
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
    static UClass* clazz = UObject::FindClassOld("Class Core.Function");
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
    static UClass* clazz = UObject::FindClassOld("Class Core.Class");
    return clazz;
  }
  SDKGEN_PADFIELD(0x228);
};
static_assert(sizeof(UClass) == 952);
