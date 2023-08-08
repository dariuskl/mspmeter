// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#include "util.hpp"

namespace meter {

  namespace {

    constexpr auto count_decimal_digits(long value) {
      constexpr auto base = 10;
      constexpr auto b2 = base * base;
      constexpr auto b3 = b2 * base;
      constexpr auto b4 = b3 * base;

      auto size = 1;

      while (true) {
        if (value < base) {
          return size;
        }
        if (value < b2) {
          return size + 1;
        }
        if (value < b3) {
          return size + 2;
        }
        if (value < b4) {
          return size + 3;
        }
        value /= b4;
        size += 4;
      }
    }

  } // namespace

  void format_number(char *const buffer, const Size field_length, int number,
                     const char padding) {
    auto *str = buffer + field_length;

    constexpr auto digits = "0001020304050607080910111213141516171819"
                            "2021222324252627282930313233343536373839"
                            "4041424344454647484950515253545556575859"
                            "6061626364656667686970717273747576777879"
                            "8081828384858687888990919293949596979899";

    while (number >= 100) {
      const auto num = (number % 100) * 2;
      number /= 100;
      --str;
      *str = digits[num + 1];
      --str;
      *str = digits[num];
    }

    if (number >= 10) {
      const auto num = number * 2;
      --str;
      *str = digits[num + 1];
      --str;
      *str = digits[num];
    } else {
      --str;
      *str = static_cast<char>('0' + number);
    }

    while (str > buffer) {
      --str;
      *str = padding;
    }
  }

} // namespace meter
