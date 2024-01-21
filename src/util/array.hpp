// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#ifndef MSPMETER_ARRAY_HPP
#define MSPMETER_ARRAY_HPP

#include <algorithm>
#include <type_traits>

template <typename Tp_, int nm_> class Array {
  public:
    using value_type = Tp_;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using size_type = decltype(nm_);

    static_assert(nm_ > 0);

    Tp_ items_[static_cast<std::make_unsigned<Size>::type>(nm_)];

    void fill(const Tp_ &value_to_fill) {
      std::fill(begin(), end(), value_to_fill);
    }

    constexpr iterator begin() noexcept { return data(); }
    constexpr const_iterator begin() const noexcept { return data(); }
    constexpr iterator end() noexcept { return data() + nm_; }
    constexpr const_iterator end() const noexcept { return data() + nm_; }

    constexpr size_type size() const noexcept { return nm_; }
    constexpr size_type max_size() const noexcept { return nm_; }
    [[nodiscard]] constexpr bool empty() const noexcept { return false; }

    constexpr reference operator[](const size_type index) noexcept {
      return items_[index];
    }

    constexpr const_reference operator[](const size_type index) const noexcept {
      return items_[index];
    }

    constexpr reference front() noexcept { return *begin(); }
    constexpr const_reference front() const noexcept { return *begin(); }
    constexpr reference back() noexcept { return *(end() - 1); }
    constexpr const_reference back() const noexcept { return *(end() - 1); }

    constexpr pointer data() { return items_; }
    constexpr const_pointer data() const { return items_; }
};

template <typename Tp_, int nm_>
constexpr bool operator==(const Array<Tp_, nm_> &one,
                          const Array<Tp_, nm_> &two) {
  return std::equal(one.begin(), one.end(), two.begin());
}

/// slice of an Array, similar to std::span, but no dynamic extent
template <typename Tp_, int nm_> class Slice {
  public:
    using element_type = Tp_;
    using value_type = std::remove_cv_t<Tp_>;
    using difference_type = std::ptrdiff_t;
    using pointer = Tp_ *;
    using const_pointer = const Tp_ *;
    using reference = element_type &;
    using const_reference = const element_type &;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using size_type = decltype(nm_);

    constexpr explicit Slice(pointer ptr) noexcept : ptr_{ptr} {}

    constexpr explicit Slice(Array<Tp_, nm_> &arr) noexcept
        : ptr_{arr.data()} {}

    template <size_type array_size>
    constexpr explicit Slice(Array<Tp_, array_size> &arr) noexcept
      requires(array_size >= nm_)
        : ptr_{arr.data()} {}

    constexpr iterator begin() noexcept { return data(); }
    constexpr const_iterator begin() const noexcept { return data(); }
    constexpr iterator end() noexcept { return data() + nm_; }
    constexpr const_iterator end() const noexcept { return data() + nm_; }

    constexpr size_type size() const noexcept { return nm_; }
    constexpr size_type max_size() const noexcept { return nm_; }
    [[nodiscard]] constexpr bool empty() const noexcept { return false; }

    constexpr reference operator[](const size_type index) noexcept {
      return ptr_[index];
    }

    constexpr const_reference operator[](const size_type index) const noexcept {
      return ptr_[index];
    }

    constexpr reference front() noexcept { return *begin(); }
    constexpr const_reference front() const noexcept { return *begin(); }
    constexpr reference back() noexcept { return *(end() - 1); }
    constexpr const_reference back() const noexcept { return *(end() - 1); }

    constexpr pointer data() { return ptr_; }
    constexpr const_pointer data() const { return ptr_; }

  private:
    pointer ptr_;
};

template <int offset_ = 0, int nm_, typename Tp_, int array_size_>
Slice<Tp_, nm_> slice(Array<Tp_, array_size_> &arr)
  requires(array_size_ >= offset_ + nm_)
{
  return Slice<Tp_, nm_>{arr.data() + offset_};
}

#endif // MSPMETER_ARRAY_HPP
