// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#ifndef MSPMETER_UTIL_7SEGMENT_HPP
#define MSPMETER_UTIL_7SEGMENT_HPP

#include "safeint.hpp"

namespace meter {

  /// Converts the given character into a bitmap for driving a 7-segment
  /// display.
  u8 to_7segment(char character);

  /// Converts the given character into a bitmap for driving a 7-segment
  /// display with a decimal point.
  u8 to_7segment(char character, bool decimal_point);

  /// Converts a string of characters into a string of bitmaps for driving a
  /// 7-segment readout.
  ///   A decimal point is appended to the preceding character.
  template <Size display_size, Size buffer_size>
  void to_7segment(Slice<u8, display_size> display_buffer,
                   const Array<char, buffer_size> &text_buffer) {
    const auto *str = text_buffer.begin();
    for (auto i = 0; i < display_size; ++i) {
      if (*(str + 1) == '.') {
        display_buffer[i] = ~to_7segment(*str, true);
        ++str;
      } else {
        display_buffer[i] = ~to_7segment(*str);
      }
      ++str;
    }
  }

} // namespace meter

#endif // MSPMETER_UTIL_7SEGMENT_HPP
