// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// See: https://github.com/microsoft/STL/blob/vs-2022-17.14/stl/inc/vector
#pragma once

template <class Array>
class TArrayConstIterator {
 public:
  using iterator_concept = std::contiguous_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;
  using value_type = typename Array::value_type;
  using difference_type = typename Array::difference_type;
  using pointer = typename Array::const_pointer;
  using reference = const value_type&;

  using Tptr = typename Array::pointer;

  constexpr TArrayConstIterator() noexcept : data_() {}
  constexpr TArrayConstIterator(Tptr arg, const Array* array) noexcept : data_(arg) { (void)array; }

  [[nodiscard]] constexpr reference operator*() const noexcept { return *data_; }
  [[nodiscard]] constexpr pointer operator->() const noexcept { return data_; }

  // forward_iterator
  constexpr TArrayConstIterator& operator++() noexcept {
    ++data_;
    return *this;
  }
  constexpr TArrayConstIterator operator++(int) noexcept {
    TArrayConstIterator tmp = *this;
    ++*this;
    return tmp;
  }

  // bidirectional_iterator
  constexpr TArrayConstIterator& operator--() noexcept {
    --data_;
    return *this;
  }
  constexpr TArrayConstIterator operator--(int) noexcept {
    TArrayConstIterator tmp = *this;
    --*this;
    return tmp;
  }

  // random_access_iterator
  constexpr void VerifyOffset(const difference_type off) const noexcept {
#if _ITERATOR_DEBUG_LEVEL == 0
    (void)off;
#else   // ^^^ _ITERATOR_DEBUG_LEVEL == 0 / _ITERATOR_DEBUG_LEVEL != 0 vvv
    // const auto _Mycont = static_cast<const Array*>(this->_Getcont());
    // _STL_VERIFY(off == 0 || _Ptr, "cannot seek value-initialized TArray iterator");
    // _STL_VERIFY(off == 0 || _Mycont, "cannot seek invalidated TArray iterator");
    // if (off < 0) {
    //   _STL_VERIFY(off >= _Mycont->_Myfirst - _Ptr, "cannot seek TArray iterator before begin");
    // }
    //
    // if (off > 0) {
    //   _STL_VERIFY(off <= _Mycont->_Mylast - _Ptr, "cannot seek TArray iterator after end");
    // }
#endif  // ^^^ _ITERATOR_DEBUG_LEVEL != 0 ^^^
  }

  constexpr TArrayConstIterator& operator+=(const difference_type off) noexcept {
    VerifyOffset(off);
    data_ += off;
    return *this;
  }

  [[nodiscard]] constexpr TArrayConstIterator operator+(const difference_type off) const noexcept {
    TArrayConstIterator tmp = *this;
    tmp += off;
    return tmp;
  }

  [[nodiscard]] friend constexpr TArrayConstIterator operator+(const difference_type off,
                                                               TArrayConstIterator next) noexcept {
    next += off;
    return next;
  }

  constexpr TArrayConstIterator& operator-=(const difference_type off) noexcept {
    return *this += -off;
  }

  [[nodiscard]] constexpr TArrayConstIterator operator-(const difference_type off) const noexcept {
    TArrayConstIterator tmp = *this;
    tmp -= off;
    return tmp;
  }

  [[nodiscard]] constexpr difference_type operator-(
      const TArrayConstIterator& right) const noexcept {
    Compat(right);
    return static_cast<difference_type>(data_ - right.data_);
  }

  [[nodiscard]] constexpr reference operator[](const difference_type off) const noexcept {
    return *(*this + off);
  }

  [[nodiscard]] constexpr bool operator==(const TArrayConstIterator& right) const noexcept {
    Compat(right);
    return data_ == right.data_;
  }

  [[nodiscard]] constexpr std::strong_ordering operator<=>(
      const TArrayConstIterator& right) const noexcept {
    Compat(right);
    return std::_Unfancy_maybe_null(data_) <=> std::_Unfancy_maybe_null(right.data_);
  }

  constexpr void Compat(const TArrayConstIterator& right) const noexcept {
    // test for compatible iterator pair
    // #if _ITERATOR_DEBUG_LEVEL == 0
    //     (void)_Right;
    // #else   // ^^^ _ITERATOR_DEBUG_LEVEL == 0 / _ITERATOR_DEBUG_LEVEL != 0 vvv
    //     _STL_VERIFY(this->_Getcont() == _Right._Getcont(), "vector iterators incompatible");
    // #endif  // ^^^ _ITERATOR_DEBUG_LEVEL != 0 ^^^
  }

 private:
  Tptr data_;
};

template <class Array>
class TArrayIterator : public TArrayConstIterator<Array> {
 public:
  using Base = TArrayConstIterator<Array>;

  using iterator_concept = std::contiguous_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;
  using value_type = typename Array::value_type;
  using difference_type = typename Array::difference_type;
  using pointer = typename Array::pointer;
  using reference = value_type&;

  using Base::Base;

  [[nodiscard]] constexpr reference operator*() const noexcept {
    return const_cast<reference>(Base::operator*());
  }

  [[nodiscard]] constexpr pointer operator->() const noexcept {
#if _ITERATOR_DEBUG_LEVEL != 0
    // const auto _Mycont = static_cast<const _Myvec*>(this->_Getcont());
    // _STL_VERIFY(this->_Ptr, "can't dereference value-initialized vector iterator");
    // _STL_VERIFY(_Mycont, "can't dereference invalidated vector iterator");
    // _STL_VERIFY(_Mycont->_Myfirst <= this->_Ptr && this->_Ptr < _Mycont->_Mylast,
    //             "can't dereference out of range vector iterator");
#endif  // _ITERATOR_DEBUG_LEVEL != 0

    return this->data_;
  }

  constexpr TArrayIterator& operator++() noexcept {
    Base::operator++();
    return *this;
  }

  constexpr TArrayIterator operator++(int) noexcept {
    TArrayIterator tmp = *this;
    Base::operator++();
    return tmp;
  }

  constexpr TArrayIterator& operator--() noexcept {
    Base::operator--();
    return *this;
  }

  constexpr TArrayIterator operator--(int) noexcept {
    TArrayIterator tmp = *this;
    Base::operator--();
    return tmp;
  }

  constexpr TArrayIterator& operator+=(const difference_type off) noexcept {
    Base::operator+=(off);
    return *this;
  }

  [[nodiscard]] constexpr TArrayIterator operator+(const difference_type off) const noexcept {
    TArrayIterator tmp = *this;
    tmp += off;
    return tmp;
  }

  [[nodiscard]] friend constexpr TArrayIterator operator+(const difference_type off,
                                                          TArrayIterator next) noexcept {
    next += off;
    return next;
  }

  constexpr TArrayIterator& operator-=(const difference_type off) noexcept {
    Base::operator-=(off);
    return *this;
  }

  using Base::operator-;

  [[nodiscard]] constexpr TArrayIterator operator-(const difference_type off) const noexcept {
    TArrayIterator tmp = *this;
    tmp -= off;
    return tmp;
  }

  [[nodiscard]] constexpr reference operator[](const difference_type off) const noexcept {
    return const_cast<reference>(Base::operator[](off));
  }
};

template <typename TElem>
class TArray {
 public:
  using Self = TArray;

  using value_type = TElem;
  using size_type = std::int32_t;
  using difference_type = std::ptrdiff_t;
  using reference = TElem&;
  using const_reference = const TElem&;
  using pointer = TElem*;
  using const_pointer = const TElem*;
  using iterator = TArrayIterator<Self>;
  using const_iterator = TArrayConstIterator<Self>;

  TArray() : data_{nullptr}, size_{0}, capacity_{0} {}
  ~TArray() = default;

  pointer data() noexcept { return data_; }
  const_pointer data() const noexcept { return data_; }

  [[nodiscard]] constexpr iterator begin() noexcept { return iterator{data_, this}; }
  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return const_iterator{data_, this};
  }
  [[nodiscard]] constexpr iterator end() noexcept {
    return begin() + static_cast<difference_type>(size_);
  }
  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return begin() + static_cast<difference_type>(size_);
  }
  [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    return const_cast<const Self&>(*this).begin();
  }
  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return const_cast<const Self&>(*this).end();
  }

  bool operator==(const Self& other) const noexcept {
    return std::equal(begin(), end(), other.begin(), other.end());
  }

  [[nodiscard]] constexpr size_type size() const noexcept {
    // Returns the number of elements of v.
    return size_;
  }
  [[nodiscard]] constexpr size_type max_size() const noexcept {
    // Returns the number of elements of the largest possible container of type C.
    return static_cast<size_type>(std::_Max_limit<difference_type>());
  }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return capacity_; }

  [[nodiscard]] bool empty() const noexcept { return begin() == end(); }

  [[nodiscard]] constexpr const_reference at(size_type off) const {
    if (size() <= off) {
      throw std::out_of_range("invalid TArray offset");
    }

    return data_[off];
  }

  [[nodiscard]] constexpr reference at(size_type off) {
    if (size() <= off) {
      throw std::out_of_range("invalid TArray offset");
    }

    return data_[off];
  }

  [[nodiscard]] constexpr const_reference operator[](size_type off) const noexcept {
    // #if _MSVC_STL_HARDENING_VECTOR || _ITERATOR_DEBUG_LEVEL != 0
    //     _STL_VERIFY(off < this->size_, "TArray subscript out of range");
    // #endif

    return data_[off];
  }

  [[nodiscard]] constexpr reference operator[](size_type off) noexcept {
    // #if _MSVC_STL_HARDENING_VECTOR || _ITERATOR_DEBUG_LEVEL != 0
    //     _STL_VERIFY(off < this->size_, "TArray out of range");
    // #endif

    return data_[off];
  }

  [[nodiscard]] constexpr reference front() noexcept {
    // #if _MSVC_STL_HARDENING_VECTOR || _ITERATOR_DEBUG_LEVEL != 0
    //     _STL_VERIFY(this->size_ != 0, "front() called on empty TArray");
    // #endif

    return *data_;
  }

  [[nodiscard]] constexpr const_reference front() const noexcept {
    // #if _MSVC_STL_HARDENING_VECTOR || _ITERATOR_DEBUG_LEVEL != 0
    //     _STL_VERIFY(this->size_ != 0, "front() called on empty TArray");
    // #endif

    return *data_;
  }

  [[nodiscard]] constexpr reference back() noexcept {
    // #if _MSVC_STL_HARDENING_VECTOR || _ITERATOR_DEBUG_LEVEL != 0
    //     _STL_VERIFY(this->size_ != 0, "back() called on empty TArray");
    // #endif

    return data_[size() - 1];
  }

  [[nodiscard]] constexpr const_reference back() const noexcept {
    // #if _MSVC_STL_HARDENING_VECTOR || _ITERATOR_DEBUG_LEVEL != 0
    //     _STL_VERIFY(this->size_ != 0, "back() called on empty TArray");
    // #endif

    return data_[size() - 1];
  }

 private:
  TElem* data_;
  size_type size_;
  size_type capacity_;
};
