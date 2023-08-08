// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#include "7segment.hpp"

namespace meter {

  //   -a-
  // f|   |b
  //   -g-
  // e|   |c
  //   -d- o DP
  constexpr auto a = u8{0x20U};
  constexpr auto b = u8{0x10U};
  constexpr auto c = u8{0x02U};
  constexpr auto d = u8{0x04U};
  constexpr auto e = u8{0x08U};
  constexpr auto f = u8{0x80U};
  constexpr auto g = u8{0x40U};
  constexpr auto DP = u8{0x01U};

  u8 to_7segment(const char character) {
    switch (character) {
    case '-':
      return g;
    case '0':
    case 'O':
      return a | b | c | d | e | f;
    case '1':
      return b | c;
    case '2':
      return a | b | d | e | g;
    case '3':
      return a | b | c | d | g;
    case '4':
      return b | c | f | g;
    case '5':
    case 'S':
      return a | c | d | f | g;
    case '6':
      return a | c | d | e | f | g;
    case '7':
      return a | b | c;
    case '8':
      return a | b | c | d | e | f | g;
    case '9':
      return a | b | c | d | f | g;
    case 'A':
      return a | b | c | e | f | g;
    case 'C':
      return a | d | e | f;
    case 'E':
      return a | d | e | f | g;
    case 'F':
      return a | e | f | g;
    case 'H':
      return b | c | e | f | g;
    case 'L':
      return d | e | f;
    case 'P':
      return a | b | e | f | g;
    case 'U':
      return b | c | d | e | f;
    case 'Y':
      return b | c | f | g;
    case 'b':
      return c | d | e | f | g;
    case 'c':
      return d | e | g;
    case 'd':
      return b | c | d | e | g;
    case 'f':
      return a | e | f | g;
    case 'h':
      return c | e | f | g;
    case 'i':
      return e;
    case 'j':
      return c | d;
    case 'n':
      return c | e | g;
    case 'o':
      return c | d | e | g;
    case 'r':
      return e | g;
    case 't':
      return d | e | f | g;
    case 'u':
      return c | d | e;
    default:
      return {};
    }
  }

  u8 to_7segment(const char character, const bool decimal_point) {
    return to_7segment(character) | (decimal_point ? DP : u8{0U});
  }

} //
