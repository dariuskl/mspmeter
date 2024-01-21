// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#ifndef MSPMETER_FUTURE_HPP
#define MSPMETER_FUTURE_HPP

#include <type_traits>
#include <version>

#ifndef __cpp_lib_to_underlying
namespace std {
  template <typename Tp_>
  constexpr auto to_underlying(Tp_ enumerator) -> underlying_type_t<Tp_> {
    return static_cast<underlying_type_t<Tp_>>(enumerator);
  }
} // namespace std
#endif

#endif // MSPMETER_FUTURE_HPP
