// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#ifndef MSPMETER_PRINT_HPP
#define MSPMETER_PRINT_HPP

#include "util.hpp"

#include <charconv>

namespace meter {

  /// integral power of 10
  constexpr auto ipow10(int exponent) {
    constexpr auto base = 10;
    auto result = 1;
    while (exponent >= 1) {
      result *= base;
      --exponent;
    }
    return result;
  }
  static_assert(ipow10(0) == 1);
  static_assert(ipow10(1) == 10);

  /// Prints a `number` right-aligned and padded to `field_length` with the
  /// defined `padding` character.
  void format_number(char *buffer, Size field_length, int number,
                     char padding = '0');

  /**
   * Formats the given number according to the given number of integral and
   * decimal digits. The resolution of the given number must be according to
   * these specifications.
   * \tparam integral_digits
   * \tparam fractional_digits
   * \param [in] number The number to be represented as a formatted string.
   * \return A buffer containing the resulting string.
   */
  template <int integral_digits, int fractional_digits, Size max_digits>
    requires(max_digits >= integral_digits + fractional_digits
                               + (fractional_digits > 0 ? 1 : 0) /* DP */
                               + 1 /* NULL */)
  void format_readout(Slice<char, max_digits> buffer, const int number) {
    const auto resolution = ipow10(fractional_digits);
    const auto magnitude = ipow10(integral_digits);

    const auto integral = std::abs(number / resolution);
    const auto fractional = std::abs(number) - (integral * resolution);

    std::fill_n(buffer.begin(), buffer.size(), '\0');

    if (number == 0) {
      for (auto i = 0; i < integral_digits - 1; ++i) {
        buffer[i] = ' ';
      }
      buffer[integral_digits - 1] = '0';
      for (auto i = 0; i < fractional_digits; ++i) {
        buffer[integral_digits + 1 + i] = '0';
      }
    } else {
      if (((number > 0) && (integral < magnitude))
          || ((number < 0) && (integral < (magnitude / 10)))) {
        format_number(buffer.data(), integral_digits, integral, ' ');
        if (fractional_digits > 0) {
          format_number(buffer.data() + integral_digits + 1, fractional_digits,
                        fractional);
        }
      } else {
        // indicate overload
        buffer[0] = ' ';
        buffer[1] = ' ';
        buffer[2] = ' ';
        buffer[3] = 'O';
        buffer[4] = 'L';
      }
    }

    if (fractional_digits > 0) {
      buffer[integral_digits] = '.';
    }

    if (number < 0) {
      buffer[0] = '-';
    }
  }

  inline Size print(char *const buffer, Size const buffer_length,
                    const int16_t value) {
    const auto result = std::to_chars(buffer, buffer + buffer_length, value);
    if (result.ec == std::errc{}) {
      return result.ptr - buffer;
    }
    return 0;
  }

  inline Size print(char *const buffer, Size const buffer_length,
                    const int32_t value) {
    const auto result = std::to_chars(buffer, buffer + buffer_length, value);
    if (result.ec == std::errc{}) {
      return result.ptr - buffer;
    }
    return 0;
  }

  inline Size print(char *const buffer, Size const buffer_length,
                    const char *const string) {
    auto i = 0;
    for (; i < buffer_length; ++i) {
      if (i < (buffer_length - 1)) {
        buffer[i] = string[i];
      } else {
        buffer[i] = '\0';
      }
      if (string[i] == '\0') {
        return i;
      }
    }
    return i;
  }

  template <Size string_length_>
  inline Size print(char *const buffer, Size const buffer_length,
                    Array<char, string_length_> const &string) {
    return print(buffer, buffer_length, string.data());
  }

  /// \return On success, the number of characters actually written.
  ///         On error, -1.
  template <typename Head_type_, typename... Tail_types_>
  inline Size print(char *const buffer, const Size buffer_length,
                    const Head_type_ &head, const Tail_types_ &...tail) {
    static_assert(sizeof...(Tail_types_) > 0,
                  "there is no implementation of print for `head`");

    auto const used_characters = print(buffer, buffer_length, head);

    if ((used_characters < 0) or (used_characters >= buffer_length)) {
      return -1;
    }

    return print(buffer + used_characters, buffer_length - used_characters,
                 tail...)
           + used_characters;
  }

  template <Size buffer_length_, typename... Args_>
  inline Size print(Array<char, buffer_length_> &buffer, const Args_ &...args) {
    return print(buffer.data(), buffer_length_, args...);
  }

} // namespace meter

#endif // MSPMETER_PRINT_HPP
