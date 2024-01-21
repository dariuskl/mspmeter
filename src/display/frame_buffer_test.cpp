// Copyright (C) 2023 SEGULA Technologies. All rights reserved.

#include "frame_buffer.hpp"

#include "test.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace medref::dcu {

  SCENARIO("rectangular fill operations") {
    GIVEN("a frame buffer covering 10×10 pixels") {
      auto buffer_bytes = std::array<std::byte, 20U>{};
      auto frame_buffer = Binary_frame_buffer{{10, 10}, buffer_bytes};

      WHEN("filling the buffer") {
        frame_buffer.fill_all();
        THEN("all bytes consist of ones") {
          REQUIRE(std::all_of(begin(buffer_bytes), end(buffer_bytes),
                              [](const auto byte) { return byte == 0xff_b; }));
        }

        AND_WHEN("clearing the buffer") {
          frame_buffer.clear_all();
          THEN("all bytes consist of zeros") {
            REQUIRE(std::all_of(begin(buffer_bytes), end(buffer_bytes),
                                [](const auto byte) { return byte == 0_b; }));
          }
        }
      }

      WHEN("drawing a rectangle of zero width") {
        frame_buffer.fill({{1, 1}, {0, 2}});
        THEN("frame buffer is unmodified") {
          REQUIRE(std::all_of(begin(buffer_bytes), end(buffer_bytes),
                              [](const auto byte) { return byte == 0_b; }));
        }
      }

      WHEN("drawing a rectangle of zero height") {
        frame_buffer.fill({{1, 1}, {2, 0}});
        THEN("frame buffer is unmodified") {
          REQUIRE(std::all_of(begin(buffer_bytes), end(buffer_bytes),
                              [](const auto byte) { return byte == 0_b; }));
        }
      }

      WHEN("drawing a rectangle of zero area") {
        frame_buffer.fill({{1, 1}, {0, 0}});
        THEN("frame buffer is unmodified") {
          REQUIRE(std::all_of(begin(buffer_bytes), end(buffer_bytes),
                              [](const auto byte) { return byte == 0_b; }));
        }
      }

      WHEN("drawing a rectangle fully outside the visible area") {
        const auto rect = GENERATE(Rect{{-5, -5}, {5, 5}},  // -x, -y
                                   Rect{{2, -5}, {5, 5}},   //  x, -y
                                   Rect{{10, -5}, {5, 5}},  // +x, -y
                                   Rect{{-5, 2}, {5, 5}},   // -x,  y
                                   Rect{{10, 2}, {5, 5}},   // +x,  y
                                   Rect{{-5, 10}, {5, 5}},  // -x, +y
                                   Rect{{2, 10}, {5, 5}},   //  x, +y
                                   Rect{{10, 10}, {5, 5}}); // +x, +y
        frame_buffer.fill(rect);
        THEN("frame buffer is unmodified") {
          CAPTURE(rect);
          REQUIRE(std::all_of(begin(buffer_bytes), end(buffer_bytes),
                              [](const auto byte) { return byte == 0_b; }));
        }
      }

      WHEN("drawing a rectangle partly outside the visible area (-x, -y)") {
        frame_buffer.fill({{-2, -2}, {4, 4}});
        THEN("rect is clipped accordingly") {
          CHECK(buffer_bytes[0] == 3_b);
          CHECK(buffer_bytes[1] == 3_b);
          REQUIRE(std::all_of(begin(buffer_bytes) + 2, end(buffer_bytes),
                              [](const auto byte) { return byte == 0_b; }));
        }
      }

      WHEN("drawing a rectangle partly outside the visible area (x, -y)") {
        frame_buffer.fill({{0, -2}, {4, 4}});
        THEN("rect is clipped accordingly") {
          CHECK(buffer_bytes
                == std::to_array({
                    3_b, 3_b, 3_b, 3_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, // 0
                    0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b  // 8
                }));
        }
      }

      WHEN("drawing a rectangle partly outside the visible area (+x, -y)") {
        frame_buffer.fill({{8, -2}, {4, 4}});
        THEN("rect is clipped accordingly") {
          CHECK(buffer_bytes
                == std::to_array({
                    0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 3_b, 3_b, // 0
                    0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b  // 8
                }));
        }
      }

      WHEN("drawing a rectangle partly outside the visible area (-x, y)") {
        frame_buffer.fill({{-2, 0}, {4, 4}});
        THEN("rect is clipped accordingly") {
          CHECK(buffer_bytes
                == std::to_array({
                    0xf_b, 0xf_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, // 0
                    0_b,   0_b,   0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b  // 8
                }));
        }
      }

      WHEN("drawing a rectangle partly outside the visible area (+x, y)") {
        frame_buffer.fill({{8, 0}, {4, 4}});
        THEN("rect is clipped accordingly") {
          CHECK(buffer_bytes
                == std::to_array({
                    0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0xf_b, 0xf_b, // 0
                    0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b,   0_b    // 8
                }));
        }
      }

      WHEN("drawing a rectangle partly outside the visible area (-x, +y)") {
        frame_buffer.fill({{-2, 8}, {4, 4}});
        THEN("rect is clipped accordingly") {
          CHECK(buffer_bytes
                == std::to_array({
                    0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, // 0
                    3_b, 3_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b  // 8
                }));
        }
      }

      WHEN("drawing a rectangle partly outside the visible area (x, +y)") {
        frame_buffer.fill({{0, 8}, {4, 4}});
        THEN("rect is clipped accordingly") {
          CHECK(buffer_bytes
                == std::to_array({
                    0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, // 0
                    3_b, 3_b, 3_b, 3_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b  // 8
                }));
        }
      }

      WHEN("drawing a rectangle partly outside the visible area (+x, +y)") {
        frame_buffer.fill({{8, 8}, {4, 4}});
        THEN("rect is clipped accordingly") {
          CHECK(buffer_bytes
                == std::to_array({
                    0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, // 0
                    0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 3_b, 3_b  // 8
                }));
        }
      }

      WHEN("drawing a rectangle over two tiles") {
        frame_buffer.fill({{0, 6}, {1, 4}});
        THEN("rect is drawn correctly") {
          CHECK(buffer_bytes
                == std::to_array({
                    0xc0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, // 0
                    3_b,    0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b  // 8
                }));
        }
      }
    }
  }

} // namespace medref::dcu
