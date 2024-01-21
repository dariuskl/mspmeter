// Copyright (C) 2023 SEGULA Technologies. All rights reserved.

#ifndef DCU_GUI_FRAME_BUFFER_HPP_
#define DCU_GUI_FRAME_BUFFER_HPP_

#include "gui/font.hpp"
#include "utils/print_scan.hpp"

#include <base/future.hpp>

#include <algorithm>
#include <span>

namespace medref::dcu {

  using coord = int;
  using extent = int;

  struct Point {
    coord x, y;
  };

  inline Ssize print(char8_t *buffer, Ssize buffer_len, const Point &value) {
    using medref::print;
    return print(buffer, buffer_len, "(", value.x, ", ", value.y, ")");
  }

  struct Dist {
    coord w, h;
  };

  inline Ssize print(char8_t *buffer, Ssize buffer_len, const Dist &value) {
    using medref::print;
    return print(buffer, buffer_len, "{", value.w, ", ", value.h, "}");
  }

  constexpr Point operator+(const Point &lhs, const Dist &rhs) {
    return Point{lhs.x + rhs.w, lhs.y + rhs.h};
  }

  constexpr Point &operator+=(Point &lhs, const Dist &rhs) {
    lhs.x += rhs.w;
    lhs.y += rhs.h;
    return lhs;
  }

  struct Rect : Point, Dist {};

  inline Ssize print(char8_t *buffer, Ssize buffer_len, const Rect &value) {
    using medref::print;
    return print(buffer, buffer_len, "{", Point{value.x, value.y}, ", ",
                 Dist{value.w, value.h}, "}");
  }

  constexpr coord right(const Rect &area) { return area.x + area.w; }
  constexpr coord bottom(const Rect &area) { return area.y + area.h; }

  enum class Alignment {
    TopLeft,
    TopCenter,
    TopRight,
    MiddleRight,
    BottomRight,
    BottomCenter,
    BottomLeft,
    MiddleLeft,
    MiddleCenter
  };

  enum class Color { Black, White };

  enum class Binary_draw_mode { Normal, Inverse };

  constexpr Binary_draw_mode operator~(const Binary_draw_mode val) {
    using enum Binary_draw_mode;
    return val == Normal ? Inverse : Normal;
  }

  /// 1bpp b/w, vertical tiles (1 byte contains 8 rows)
  class Binary_frame_buffer {
   public:
    static constexpr auto tile_size = 8;

    constexpr explicit Binary_frame_buffer(Dist extents,
                                           std::span<std::byte> buffer)
        : m_buffer{buffer.subspan(
            0, static_cast<uint32_t>(extents.w
                                     * utils::ceil_div(extents.h, tile_size)))},
          m_extents{extents} {}

    constexpr auto bytes() { return m_buffer.data(); }

    auto &set_mode(const Binary_draw_mode draw_mode) {
      m_mode = draw_mode;
      return *this;
    }

    auto &set_font(const Font *font) {
      m_font = font;
      return *this;
    }

    /// Fills the entire buffer with the foreground color.
    /// Cursor is not modified.
    Binary_frame_buffer &fill_all();

    /// Fills the entire buffer with the background color.
    /// Cursor is not modified.
    Binary_frame_buffer &clear_all();

    /// Fills the given rectangle with the foreground color.
    /// Cursor is not modified.
    auto &fill(const Rect &rect) {
      fill_(rect, m_mode);
      return *this;
    }

    /// Fills the given rectangle with the background color.
    /// Cursor is not modified.
    auto &clear(const Rect &rect) {
      fill_(rect, ~m_mode);
      return *this;
    }

    /// Draws the outline of the given rectangle within its extents.
    /// Cursor is not modified.
    auto &draw_rect(const Rect &rect) {
      line_({rect.x, rect.y}, {rect.x + rect.w - 1, rect.y});
      line_({rect.x + rect.w - 1, rect.y},
            {rect.x + rect.w - 1, rect.y + rect.h - 1});
      line_({rect.x, rect.y + rect.h - 1},
            {rect.x + rect.w - 1, rect.y + rect.h - 1});
      line_({rect.x, rect.y}, {rect.x, rect.y + rect.h - 1});
      return *this;
    }

    /// Moves the cursor to the given position.
    auto &at(const Point &pos) {
      m_pos = pos;
      return *this;
    }

    /// Moves the cursor by the given distance.
    auto &move(const Dist &delta) {
      m_pos += delta;
      return *this;
    }

    /// Fills the rectangle from the current position by the given distance with
    /// the foreground color.
    /// Cursor is set to the end point of the rectangle (bottom right).
    auto &fill(const Dist &area) {
      return fill({m_pos, area}).at(m_pos + area);
    }

    /// Fills the rectangle from the current position by the given distance with
    /// the background color.
    /// Cursor is set to the end point of the rectangle (bottom right).
    auto &clear(const Dist &area) {
      return clear({m_pos, area}).at(m_pos + area);
    }

    /// Draws a line from the current position to the given endpoint. Cursor is
    /// set to the endpoint.
    auto &line_to(const Point &end) {
      line_(m_pos, end);
      return at(end);
    }

    auto &hline_to(const coord end_x) {
      line_(m_pos, {end_x, m_pos.y});
      return at({end_x, m_pos.y});
    }

    auto &vline_to(const coord end_y) {
      line_(m_pos, {m_pos.x, end_y});
      return at({m_pos.x, end_y});
    }

    /// Prints the given string at the current position.
    auto &print(const char8_t *const str,
                const Alignment alignment = Alignment::TopLeft,
                const extent max_line_length = -1,
                const extent max_height = -1) {
      print_(str, alignment,
             max_line_length >= 0 ? max_line_length : m_extents.w - m_pos.x,
             max_height >= 0 ? max_height : m_extents.h - m_pos.y);
      return *this;
    }

   private:
    void set_(const Point &pos);
    void clear_(const Point &pos);
    void line_(const Point &start, const Point &end);
    void fill_(const Rect &rect, Binary_draw_mode draw_mode);

    void print_(const char8_t *str_to_print, Alignment alignment,
                extent max_line_length, extent max_height);
    extent print_(char32_t a_char);

    void bitmap(const std::byte *bitmap, const Point &pos, const Dist &extents);

    std::span<std::byte> m_buffer;
    Dist m_extents;
    Point m_pos{0, 0};
    const Font *m_font{nullptr};
    Binary_draw_mode m_mode{};
  };

} // namespace medref::dcu

#endif // DCU_GUI_FRAME_BUFFER_HPP_
