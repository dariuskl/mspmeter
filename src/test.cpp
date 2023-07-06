// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

void __delay_cycles([[maybe_unused]] int cycles) {}

#include "msp430i2.hpp"
#include "util.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <format>
#include <string_view>

namespace Catch {
  template <> struct StringMaker<u8> {
      static std::string convert(u8 const value) {
        return std::format("{:#02x}", std::to_underlying(value));
      }
  };
} // namespace Catch

namespace meter {

  SCENARIO("calculate device descriptor checksum") {
    // taken from a real device
    const auto descriptor = msp430i2::Device_descriptor_and_checksum{
        .checksum = u16{0x4adbU},
        .words = {{u16{0x0a01U}, u16{0x8851U}, u16{0x5172U}, u16{0x000eU},
                   u16{0x0014U}, u16{0xfef8U}, u16{0x0202U}, u16{0x3e00U},
                   u16{0x0403U}, u16{0xc084U}, u16{0x4069U}, u16{0x0204U},
                   u16{0xff0cU}, u16{0x22feU}, u16{0xffffU}, u16{0xffffU},
                   u16{0xffffU}, u16{0xffffU}, u16{0xffffU}, u16{0xffffU},
                   u16{0xffffU}, u16{0xffffU}, u16{0xffffU}, u16{0xffffU},
                   u16{0xffffU}, u16{0xffffU}, u16{0xffffU}, u16{0xffffU},
                   u16{0xffffU}, u16{0xffffU}, u16{0xffffU}}}};
    REQUIRE(msp430i2::calculate_device_descriptor_checksum(descriptor.words)
            == descriptor.checksum);
  }

  SCENARIO("ADC result to voltage conversion") {
    // TODO
  }

  SCENARIO("slicing") {
    GIVEN("an array of eight distinct elements") {
      auto buffer = Array<char, 8>{'1', '2', '3', '4', '5', '6', '7', '8'};
      WHEN("slicing in two slices of four each") {
        auto slice1 = slice<0, 4>(buffer);
        auto slice2 = slice<4, 4>(buffer);
        THEN("slice contents are as expected") {
          CHECK(slice1[0] == '1');
          CHECK(slice1[1] == '2');
          CHECK(slice1[2] == '3');
          CHECK(slice1[3] == '4');
          CHECK(slice2[0] == '5');
          CHECK(slice2[1] == '6');
          CHECK(slice2[2] == '7');
          CHECK(slice2[3] == '8');
        }
      }
    }
  }

  using namespace std::string_view_literals;

  SCENARIO("print for serial") {
    auto buffer = Array<char, 32>{};

    GIVEN("") {
      CHECK(print(buffer.begin(), buffer.size(), "30.00\t1.000\r\n") == 13);
      REQUIRE(buffer.data() == "30.00\t1.000\r\n"sv);
    }
  }

  SCENARIO("readout formatting for voltage (xx.xx)") {
    auto buffer = Array<char, 6>{};

    GIVEN("positive overload") {
      const auto number = 100'00;
      WHEN("formatting") {
        format_readout<2, 2>(buffer, number);
        THEN("OL shown") { REQUIRE(buffer.data() == "  .OL"sv); }
      }
    }

    GIVEN("positive full-scale") {
      const auto number = 99'99;
      WHEN("formatting") {
        format_readout<2, 2>(buffer, number);
        THEN("one leading zero") { REQUIRE(buffer.data() == "99.99"sv); }
      }
    }

    GIVEN("zero") {
      const auto number = 0;
      WHEN("formatting") {
        format_readout<2, 2>(buffer, number);
        THEN("one leading zero") { REQUIRE(buffer.data() == " 0.00"sv); }
      }
    }

    GIVEN("a negative value < 10") {
      const auto number = -999;
      WHEN("formatting") {
        format_readout<2, 2>(buffer, number);
        THEN("minus sign is prepended") { REQUIRE(buffer.data() == "-9.99"sv); }
      }
    }

    GIVEN("a negative value > 10") {
      const auto number = -1000;
      WHEN("formatting") {
        format_readout<2, 2>(buffer, number);
        THEN("minus sign is prepended") { REQUIRE(buffer.data() == "- .OL"sv); }
      }
    }
  }

  SCENARIO("readout formatting for current (x.xxx)") {
    auto buffer = Array<char, 6>{};

    GIVEN("positive overload") {
      const auto number = 10'000;
      WHEN("formatting") {
        format_readout<1, 3>(buffer, number);
        THEN("OL shown") { REQUIRE(buffer.data() == " . OL"sv); }
      }
    }

    GIVEN("positive full-scale") {
      const auto number = 9'999;
      WHEN("formatting") {
        format_readout<1, 3>(buffer, number);
        THEN("one leading zero") { REQUIRE(buffer.data() == "9.999"sv); }
      }
    }

    GIVEN("zero") {
      const auto number = 0;
      WHEN("formatting") {
        format_readout<1, 3>(buffer, number);
        THEN("one leading zero") { REQUIRE(buffer.data() == "0.000"sv); }
      }
    }

    GIVEN("a negative value < 1") {
      const auto number = -999;
      WHEN("formatting for x.xxx") {
        format_readout<1, 3>(buffer, number);
        THEN("minus sign is in place of leading zero") {
          REQUIRE(buffer.data() == "-.999"sv);
        }
      }
    }

    GIVEN("a negative value > 1") {
      const auto number = -1000;
      WHEN("formatting") {
        format_readout<1, 3>(buffer, number);
        THEN("minus sign is prepended") { REQUIRE(buffer.data() == "-. OL"sv); }
      }
    }
  }

  SCENARIO("readout formatting for integral values (xxxx)") {
    auto buffer = Array<char, 6>{};

    GIVEN("zero") {
      const auto number = 0;
      WHEN("formatting") {
        format_readout<4, 0>(buffer, number);
        THEN("one leading zero") { REQUIRE(buffer.data() == "   0"sv); }
      }
    }

    GIVEN("a negative value") {
      const auto number = -1;
      WHEN("formatting for xxxx") {
        format_readout<4, 0>(buffer, number);
        THEN("") {
          REQUIRE(buffer.data() == "-  1"sv);
        }
      }
    }
  }

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
