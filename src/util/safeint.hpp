// Darius Kellermann <kellermann@pm.me>

#ifndef SAFEINT_HPP_
#define SAFEINT_HPP_

#include "future.hpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>

enum class u8 : uint8_t {};

#ifdef __cpp_consteval
consteval
#else
constexpr
#endif
    u8
    operator""_u8(long long unsigned const v) {
  if (v <= std::numeric_limits<std::underlying_type_t<std::byte>>::max()) {
    return static_cast<u8>(v);
  }
#ifdef __cpp_consteval
  throw std::exception{};
#else
  return u8{0U};
#endif
}

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

#endif // SAFEINT_HPP_
