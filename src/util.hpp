// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <version>

template <typename Target_type_, typename Source_type_>
  requires(std::is_integral_v<Target_type_> && std::is_integral_v<Source_type_>
           && (std::numeric_limits<Source_type_>::digits
               > std::numeric_limits<Target_type_>::digits))
Target_type_ saturate_cast(const Source_type_ val) {
  return static_cast<Target_type_>(
      std::clamp(val, Source_type_{std::numeric_limits<Target_type_>::min()},
                 Source_type_{std::numeric_limits<Target_type_>::max()}));
}

template <typename Tp_ = std::byte>
constexpr Tp_ ones(const int number_of_ones) {
  const auto digits =
      std::is_enum_v<Tp_>
          ? std::numeric_limits<std::underlying_type_t<Tp_>>::digits
          : std::numeric_limits<Tp_>::digits;
  return static_cast<Tp_>(
      (1U << static_cast<unsigned>(std::clamp(number_of_ones, 0, digits)))
      - 1U);
}

#endif // UTIL_HPP_
