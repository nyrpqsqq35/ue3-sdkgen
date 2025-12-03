//
// Created on 03-Dec-25.
// Copyright (c) 2025. All rights reserved.
//

#pragma once
#include <cstdint>

namespace bridge {

// inline std::map<uintptr_t, std::vector<uint8_t>> ptr_to_data_map;

template <typename T>
class Pointer {
  using Self = Pointer<T>;

 public:
  Pointer() : data_(0) {}
  Pointer(const uintptr_t data) : data_(data) {}  // NOLINT(*-explicit-constructor)
  Pointer(const Pointer& p) = default;            // NOLINT(*-explicit-constructor)
  Pointer(Pointer&& p) noexcept : data_(p.data_) {}
  Pointer& operator=(const Pointer&) = delete;
  Pointer& operator=(Pointer&& b) noexcept {
    if (this != &b) {
      data_ = b.data_;
      b.data_ = 0;
    }
    return *this;
  }

  [[nodiscard]] uintptr_t get() const { return data_; }

  bool IsValid() const {
    if (data_ == 0) return false;
    return true;
  }

  Self operator+(T m) { return {data_ + m}; }
  T& operator*() const { return reinterpret_cast<T&>(data_); }

  T* operator->() const { return reinterpret_cast<T*>(data_); }

  inline bool operator==(const Self& rhs) const { return this->get() == rhs.get(); }
  inline bool operator!=(const Self& rhs) const { return !(this == rhs); }

  void Set(uintptr_t p) { data_ = p; }

  template <typename H>
  friend H AbslHashValue(H h, const Self& self) {
    return H::combine(std::move(h), self.data_);
  }

 private:
  uintptr_t data_;
};

}  // namespace bridge
