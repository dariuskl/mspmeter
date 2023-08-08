// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#include "7segment.hpp"

#include <catch2/catch_test_macros.hpp>

namespace meter {

  SCENARIO("conversion to 7-segment bytes") {
    auto buffer = Array<u8, 4>{};

    GIVEN("number without decimal point") {
      const auto text = Array<char, 6>{"   0"};
      WHEN("") {
        to_7segment(Slice{buffer}, text);
        THEN("") {
          REQUIRE(buffer
                  == Array<u8, 4>{~u8{0x00}, ~u8{0x00}, ~u8{0x00}, ~u8{0xbe}});
        }
      }
    }

    GIVEN("number with decimal point") {
      const auto text = Array<char, 6>{"0.001"};
      WHEN("") {
        to_7segment(Slice{buffer}, text);
        THEN("") {
          REQUIRE(
              buffer
              == Array<u8, 4>{~u8{0xbfU}, ~u8{0xbeU}, ~u8{0xbeU}, ~u8{0x12U}});
        }
      }
    }
  }

} // namespace meter
