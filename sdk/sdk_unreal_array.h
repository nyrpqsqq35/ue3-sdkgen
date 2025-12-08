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

  constexpr TArray() : data_{nullptr}, size_{0}, capacity_{0} {}
  constexpr TArray(const Self& other) {
    if (other.size_ > 0) {
      data_ = static_cast<value_type*>(::operator new(other.capacity_ * sizeof(value_type)));
      for (size_type i = 0; i < other.size_; ++i) {
        new (data_ + i) value_type(other.data_[i]);
      }

      size_ = other.size_;
      capacity_ = other.capacity_;
    } else {
      data_ = nullptr;
      size_ = capacity_ = 0;
    }
  }
  constexpr TArray(Self&& other) noexcept
      : data_{other.data_}, size_{other.size_}, capacity_{other.capacity_} {
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }
  ~TArray() {
    clear();
    if (data_) {
      ::operator delete(data_);
    }
  }

  constexpr Self& operator=(const Self& other) {
    if (this == &other) return *this;

    clear();
    reserve(other.size_);

    for (size_type i = 0; i < other.size_; ++i) {
      new (data_ + i) value_type(other.data_[i]);
    }
    size_ = other.size_;

    return *this;
  }

  constexpr Self& operator=(Self&& other) noexcept {
    if (this == &other) return *this;

    clear();
    if (data_) ::operator delete(data_);

    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;

    other.data_ = nullptr;
    other.size_ = other.capacity_ = 0;

    return *this;
  }

  constexpr void assign(size_type count, const value_type& value) {
    clear();
    reserve(count);
    for (size_type i = 0; i < count; ++i) {
      new (data_ + i) value_type(value);
    }
    size_ = count;
  }

  template <class InputIt>
  constexpr void assign(InputIt first, InputIt last) {
    clear();
    if constexpr (std::forward_iterator<InputIt>) {
      const auto dist = static_cast<size_type>(std::distance(first, last));
      reserve(dist);
    }
    for (; first != last; ++first) {
      push_back(*first);
    }
  }

  template <class Range>
  constexpr void assign_range(Range&& range) {
    assign(std::begin(range), std::end(range));
  }

  // Element access
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
  pointer data() noexcept { return data_; }
  const_pointer data() const noexcept { return data_; }

  // Iterators
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

  // Capacity
  [[nodiscard]] bool empty() const noexcept { return begin() == end(); }
  [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
  [[nodiscard]] constexpr size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max();
  }
  constexpr void reserve(size_type new_capacity) {
    if (new_capacity > capacity_) {
      Reallocate(new_capacity);
    }
  }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return capacity_; }
  constexpr void shrink_to_fit() {
    if (size_ < capacity_) {
      if (size_ == 0) {
        if (data_) ::operator delete(data_);
        data_ = nullptr;
        capacity_ = 0;
      } else {
        Reallocate(size_);
      }
    }
  }

  constexpr void Reallocate(size_type new_capacity) {
    auto* new_data = static_cast<value_type*>(::operator new(new_capacity * sizeof(value_type)));
    const auto new_size = std::min(size_, new_capacity);

    for (size_type i = 0; i < new_size; ++i) {
      new (new_data + i) value_type(std::move(data_[i]));
    }

    for (size_type i = 0; i < size_; ++i) {
      data_[i].~value_type();
    }

    if (data_) {
      ::operator delete(data_);
    }
    data_ = new_data;
    size_ = new_size;
    capacity_ = new_capacity;
  }

  // Modifiers
  constexpr void clear() {
    for (size_type i = 0; i < size_; ++i) {
      data_[i].~value_type();
    }
    size_ = 0;
  }
  constexpr iterator insert(const_iterator pos, const value_type& value) {
    return emplace(pos, value);
  }

  constexpr iterator insert(const_iterator pos, value_type&& value) {
    return emplace(pos, std::move(value));
  }

  constexpr iterator insert(const_iterator pos, size_type count, const value_type& value) {
    const difference_type offset = pos - cbegin();
    if (count == 0) return begin() + offset;

    if (size_ + count > capacity_) {
      Reallocate(size_ + count);
    }

    size_type current_size = size_;
    size_ += count;

    for (difference_type i = current_size - 1; i >= offset; --i) {
      if (i + count >= current_size) {
        new (data_ + i + count) value_type(std::move(data_[i]));
      } else {
        data_[i + count] = std::move(data_[i]);
      }
    }

    for (size_type i = 0; i < count; ++i) {
      if (offset + i >= current_size) {
        new (data_ + offset + i) value_type(value);
      } else {
        data_[offset + i] = value;
      }
    }

    return begin() + offset;
  }

  template <class InputIt>
  constexpr iterator insert(const_iterator pos, InputIt first, InputIt last) {
    const difference_type offset = pos - cbegin();
    if (first == last) return begin() + offset;

    if constexpr (std::forward_iterator<InputIt>) {
      auto count = std::distance(first, last);
      if (size_ + count > capacity_) Reallocate(size_ + count);

      size_type current_size = size_;
      size_ += count;

      for (difference_type i = current_size - 1; i >= offset; --i) {
        if (i + count >= current_size) {
          new (data_ + i + count) value_type(std::move(data_[i]));
        } else {
          data_[i + count] = std::move(data_[i]);
        }
      }

      auto it = first;
      for (size_type i = 0; i < count; ++i, ++it) {
        if (offset + i >= current_size) {
          new (data_ + offset + i) value_type(*it);
        } else {
          data_[offset + i] = *it;
        }
      }
      return begin() + offset;
    } else {
      iterator it = begin() + offset;
      for (; first != last; ++first) {
        it = insert(it, *first);
        ++it;
      }
      return begin() + offset;
    }
  }

  template <class Range>
  constexpr iterator insert_range(const_iterator pos, Range&& range) {
    return insert(pos, std::begin(range), std::end(range));
  }
  template <class... Args>
  constexpr iterator emplace(const_iterator pos, Args&&... args) {
    const difference_type offset = pos - cbegin();

    if (size_ >= capacity_) {
      Reallocate(capacity_ == 0 ? 4 : capacity_ * 2);
    }

    pointer p = data_ + offset;

    if (p < data_ + size_) {
      new (data_ + size_) value_type(std::move(data_[size_ - 1]));
      for (pointer k = data_ + size_ - 1; k > p; --k) {
        *k = std::move(*(k - 1));
      }

      p->~value_type();
    }

    new (p) value_type(std::forward<Args>(args)...);
    size_++;

    return iterator(p, this);
  }
  constexpr iterator erase(const_iterator pos) { return erase(pos, pos + 1); }

  constexpr iterator erase(const_iterator first, const_iterator last) {
    difference_type offset_start = first - cbegin();
    difference_type offset_end = last - cbegin();
    difference_type count = offset_end - offset_start;

    if (count == 0) return begin() + offset_start;

    pointer p_dest = data_ + offset_start;
    pointer p_src = data_ + offset_end;

    pointer p_end = data_ + size_;
    while (p_src < p_end) {
      *p_dest = std::move(*p_src);
      ++p_dest;
      ++p_src;
    }

    for (size_type i = 0; i < count; ++i) {
      data_[size_ - 1 - i].~value_type();
    }

    size_ -= count;
    return begin() + offset_start;
  }
  constexpr void push_back(const value_type& value) { emplace_back(value); }
  constexpr void push_back(value_type&& value) { emplace_back(std::move(value)); }
  template <class... Args>
  constexpr reference emplace_back(Args&&... args) {
    if (size_ >= capacity_) {
      Reallocate(capacity_ == 0 ? 4 : capacity_ * 2);
    }
    new (data_ + size_) value_type(std::forward<Args>(args)...);
    return data_[size_++];
  }
  template <class Range>
  constexpr void append_range(Range&& range) {
    insert(cend(), std::begin(range), std::end(range));
  }
  constexpr void pop_back() {
    if (!empty()) {
      data_[size_ - 1].~value_type();
      --size_;
    }
  }
  constexpr void resize(size_type count) {
    if (size_ > count) {
      for (size_type i = count; i < size_; ++i) {
        data_[i].~value_type();
      }
      size_ = count;
    } else if (size_ < count) {
      if (count > capacity_) {
        Reallocate(count);
      }
      for (size_type i = size_; i < count; ++i) {
        new (data_ + i) value_type();
      }
      size_ = count;
    }  // else if (count == size_) { return; }
  }
  constexpr void resize(size_type count, const value_type& value) {
    if (size_ > count) {
      for (size_type i = count; i < size_; ++i) {
        data_[i].~value_type();
      }
      size_ = count;
    } else if (size_ < count) {
      if (count > capacity_) {
        Reallocate(count);
      }

      for (size_type i = size_; i < count; ++i) {
        new (data_ + i) value_type(value);
      }
      size_ = count;
    }  // else if (count == size_) { return; }
  }

 protected:
  TElem* data_;
  size_type size_;
  size_type capacity_;
};
