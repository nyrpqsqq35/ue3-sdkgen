//
// Created on 03-Dec-25.
// Copyright (c) 2025. All rights reserved.
//

#include "unreal.h"

// UObject
std::string UObject::GetFullName() const {
  std::string name;
  auto outer_obj = outer;

  while (outer_obj.IsValid()) {
    name = outer_obj->name.ToString() + "." + name;
    outer_obj.Set(outer_obj->outer.get());
  }
  name += this->name.ToString();

  if (this->class_.IsValid()) {
    name = this->class_->name.ToString() + " " + name;
  }

  return name;
}

std::string UObject::GetPackageName() const {
  auto outer_obj = outer;
  if (!outer_obj.IsValid()) return "";

  do {
    auto& new_outer = outer_obj->outer;
    if (!new_outer.IsValid()) break;
    outer_obj.Set(new_outer.get());
  } while (outer_obj.IsValid());

  return outer_obj->name.ToString();
}

bridge::Pointer<UObject> UObject::GetPackageObject() const {
  auto outer_obj = outer;
  if (!outer_obj.IsValid()) return 0;

  do {
    auto& new_outer = outer_obj->outer;
    if (!new_outer.IsValid()) break;
    outer_obj.Set(new_outer.get());
  } while (outer_obj.IsValid());

  return outer_obj;
}

bool UObject::IsA(bridge::Pointer<UClass> uclass) const {
  auto outer_class = class_;
  while (outer_class.IsValid()) {
    if (outer_class == uclass) return true;
    outer_class.Set(outer_class->superfield.get());
  }
  return false;
}

bool UObject::IsA(const std::string& class_name) const { return IsA(StringToClass[class_name]); }

std::string FName::ToString() const {
  auto& wstr = FNameIndexToString[fname_entry_id];
  return std::string(wstr.begin(), wstr.end());
}
std::string FString::ToString() const {
  // will assert on data.end() - 1 if count == 0
  if (count() == 0) return "<null>";

  auto data = Fetch();
  return std::string(data.begin(),
                     data.end() - 1  // remove null terminator
  );
}
