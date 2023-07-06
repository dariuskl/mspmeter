// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#ifndef FUTURE_HPP_
#define FUTURE_HPP_

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>

using Size = std::ptrdiff_t;

#ifndef __cpp_lib_to_underlying
namespace std {
  template <typename Tp_>
  constexpr auto to_underlying(Tp_ enumerator) ->
      typename std::underlying_type<Tp_>::type {
    return static_cast<typename std::underlying_type<Tp_>::type>(enumerator);
  }
} // namespace std
#endif

enum class u8 : uint8_t {};

inline constexpr u8 operator~(const u8 rhs) {
  return static_cast<u8>(~std::to_underlying(rhs));
}

inline constexpr u8 operator&(const u8 lhs, const u8 rhs) {
  return static_cast<u8>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

inline constexpr u8 operator|(const u8 lhs, const u8 rhs) {
  return static_cast<u8>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

inline constexpr u8 operator^(const u8 lhs, const u8 rhs) {
  return static_cast<u8>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
}

inline constexpr u8 operator<<(const u8 lhs, const unsigned rhs) {
  return static_cast<u8>(std::to_underlying(lhs) << rhs);
}

inline constexpr u8 operator>>(const u8 lhs, const unsigned rhs) {
  return static_cast<u8>(std::to_underlying(lhs) >> rhs);
}

enum class u16 : uint16_t {};

inline constexpr u16 operator~(const u16 lhs) {
  return static_cast<u16>(~std::to_underlying(lhs));
}

inline constexpr u16 operator&(const u16 lhs, const u16 rhs) {
  return static_cast<u16>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

inline constexpr u8 operator&(const u16 lhs, const u8 rhs) {
  return static_cast<u8>(std::to_underlying(lhs) & static_cast<uint8_t>(rhs));
}

inline constexpr u16 operator|(const u16 lhs, const u16 rhs) {
  return static_cast<u16>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

inline constexpr u16 operator|(const u16 lhs, const u8 rhs) {
  return static_cast<u16>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

inline constexpr u16 operator^(const u16 lhs, const u16 rhs) {
  return static_cast<u16>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
}

inline constexpr u16 operator>>(const u16 lhs, const unsigned rhs) {
  return static_cast<u16>(std::to_underlying(lhs) >> rhs);
}

inline constexpr u16 operator<<(const u16 lhs, const unsigned rhs) {
  return static_cast<u16>(std::to_underlying(lhs) << rhs);
}

constexpr u16 twos_complement(const u16 val) {
  return static_cast<u16>(static_cast<uint16_t>(~val) + 1U);
}

enum class u32 : uint32_t {};

inline constexpr u32 operator&(const u32 lhs, const u32 rhs) {
  return static_cast<u32>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

inline constexpr u32 operator|(const u32 lhs, const u32 rhs) {
  return static_cast<u32>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

inline constexpr u32 operator|(const u32 lhs, const u16 rhs) {
  return static_cast<u32>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

inline constexpr u32 operator|(const u32 lhs, const u8 rhs) {
  return static_cast<u32>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

inline constexpr u32 operator>>(const u32 lhs, const unsigned rhs) {
  return static_cast<u32>(std::to_underlying(lhs) >> rhs);
}

inline constexpr u32 operator<<(const u32 lhs, const unsigned rhs) {
  return static_cast<u32>(std::to_underlying(lhs) << rhs);
}

template <typename Tp_> struct Register {
    intptr_t address;
};

template <typename Tp_, typename Head_tp_> Tp_ b_or(Head_tp_ head) {
  return static_cast<Tp_>(head);
}

template <typename Tp_, typename Head_tp_, typename... Tail_tps_>
Tp_ b_or(Head_tp_ head, Tail_tps_... tail) {
  return static_cast<Tp_>(static_cast<Tp_>(head) | b_or<Tp_>(tail...));
}

template <typename Tp_> inline Tp_ load(const Register<Tp_> a_register) {
  return *reinterpret_cast<volatile Tp_ *>(a_register.address);
}

template <typename Tp_>
inline void store(const Register<Tp_> a_register, Tp_ const val) {
  *reinterpret_cast<volatile Tp_ *>(a_register.address) = val;
}

template <typename Tp_>
inline void set_bits(const Register<Tp_> a_register, Tp_ const mask) {
  store(a_register, static_cast<Tp_>(load(a_register) | mask));
}

template <typename Tp_>
inline void clear_bits(const Register<Tp_> a_register, Tp_ const mask) {
  store(a_register, static_cast<Tp_>(load(a_register) & ~mask));
}

template <typename Tp_>
inline void toggle_bits(const Register<Tp_> a_register, Tp_ const mask) {
  store(a_register, static_cast<Tp_>(load(a_register) ^ mask));
}

template <typename Tp_, Size nm_> class Array {
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

template <typename Tp_, Size nm_>
constexpr bool operator==(const Array<Tp_, nm_> &one,
                          const Array<Tp_, nm_> &two) {
  return std::equal(one.begin(), one.end(), two.begin());
}

/// slice of an Array, similar to std::span, but no dynamic extent
template <typename Tp_, Size nm_> class Slice {
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

template <Size offset_ = 0, Size nm_, typename Tp_, Size array_size_>
Slice<Tp_, nm_> slice(Array<Tp_, array_size_> &arr)
  requires(array_size_ >= offset_ + nm_)
{
  return Slice<Tp_, nm_>{arr.data() + offset_};
}

#endif // FUTURE_HPP_
