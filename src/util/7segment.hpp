// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#ifndef MSPMETER_7SEGMENT_HPP
#define MSPMETER_7SEGMENT_HPP

#include "../future.hpp"

namespace meter {

  u8 to_7segment(char character);
  u8 to_7segment(char character, bool decimal_point);

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

#endif // MSPMETER_7SEGMENT_HPP
