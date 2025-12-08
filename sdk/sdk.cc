//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#include "sdk.h"

#include <map>
#include <ranges>
#include <unordered_map>
#include "sdk_rt.h"

namespace {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-cast"  // shut up
template <typename T>
T GetVirtualFunction(const void* instance, const size_t index) {
  auto vtable = *static_cast<const void***>(const_cast<void*>(instance));
  return static_cast<T>(vtable[index]);
}
#pragma clang diagnostic pop

}  // namespace

TArray<UObject*>* GObjects;
TArray<FNameEntry*>* GNames;

std::string UObject::GetFullName() const {
  std::string full_name;
  auto outer_obj = outer;

  while (outer_obj) {
    full_name = outer_obj->name.ToString() + "." + full_name;
    outer_obj = outer_obj->outer;
  }
  full_name += this->name.ToString();

  if (class_) {
    full_name = class_->name.ToString() + " " + full_name;
  }
  return full_name;
}
std::string UObject::GetPackageName() const {
  auto outer_obj = outer;
  if (!outer_obj) return "";

  do {
    const auto& new_outer = outer_obj->outer;
    if (!new_outer) break;
    outer_obj = new_outer;
  } while (outer_obj);

  return outer_obj->name.ToString();
}

bool UObject::IsA(const UClass* uclass) const {
  for (const UClass* super_class = class_; super_class;
       super_class = static_cast<UClass*>(super_class->superfield)) {
    if (super_class == uclass) return true;
  }
  return false;
}
bool UObject::IsA(const int32_t obj_internal_integer) const {
  if (const auto clazz = GObjects()[obj_internal_integer]->class_) {
    return IsA(clazz);
  }
  return false;
}

TArray<UObject*>& UObject::GObjects() { return *::GObjects; }

TArray<FNameEntry*>& UObject::GNames() { return *::GNames; }

UClass* UObject::FindClassOld(const std::string_view class_full_name) {
  for (auto* obj : GObjects()) {
    if (obj->GetFullName() == class_full_name) {
      return static_cast<UClass*>(obj);
    }
  }
  return nullptr;
}
UClass* UObject::FindClass(const std::string_view class_full_name) {
  return FindClass(SDKGEN_NS::StringHash<>::Calculate(class_full_name));
}
UClass* UObject::FindClass(const rlsdk::StringHash<> hash) {
  return SDKGEN_NS::rt().FindClass(hash);
}
UFunction* UFunction::FindFunction(const std::string_view function_full_name) {
  return FindFunction(SDKGEN_NS::StringHash<>::Calculate(function_full_name));
}
UFunction* UFunction::FindFunction(const SDKGEN_NS::StringHash<> hash) {
  return SDKGEN_NS::rt().FindFunction(hash);
}

void UObject::ProcessEvent(class UFunction* uFunction, void* uParams, void* uResult) {
  GetVirtualFunction<void (*)(class UObject*, class UFunction*, void*)>(this, 67)(this, uFunction,
                                                                                  uParams);
}

namespace {
template <typename TGameChar>
auto GameCharToString(const TGameChar* data, const std::size_t len) {
  if constexpr (std::is_same_v<TGameChar, wchar_t>) {
    std::wstring_view sv{data, len};
    return std::string(sv.begin(), sv.end());
  } else if constexpr (std::is_same_v<TGameChar, char>) {
    return std::string_view{data, len};
  } else {
    static_assert(false, "game_char is wrong type");
  }
}

template <typename TGameChar>
auto GameCharToString(const TGameChar* data) {
  if constexpr (std::is_same_v<TGameChar, wchar_t>) {
    const auto len = wcslen(data);
    std::wstring_view sv{data, len};
    return std::string(sv.begin(), sv.end());
  } else if constexpr (std::is_same_v<TGameChar, char>) {
    const auto len = strlen(data);
    return std::string_view{data, len};
  } else {
    static_assert(false, "game_char is wrong type");
  }
}
}  // namespace

std::string FNameEntry::ToString() const { return GameCharToString(name); }
std::string FName::ToString() const { return (*GNames)[entry_id]->ToString(); }
std::string FString::ToString() const {
  // if (data() == nullptr) return "<nullptr>";
  if (size() == 0) return "";

  return GameCharToString(data(), static_cast<std::size_t>(size()));
}
