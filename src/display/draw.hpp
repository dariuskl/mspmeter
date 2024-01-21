// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#ifndef DISPLAY_DRAW_HPP_
#define DISPLAY_DRAW_HPP_

#include "util.hpp"
#include "util/array.hpp"
#include "util/safeint.hpp"

namespace meter {

  using coord = int_fast16_t;
  using extent = int_fast16_t;

  /// An absolute point in the coordinate system.
  struct Point {
      coord x, y;
  };

  /// A relative length.
  struct Dist {
      coord w, h;
  };

  constexpr Point operator+(const Point &lhs, const Dist &rhs) {
    return Point{lhs.x + rhs.w, lhs.y + rhs.h};
  }

  constexpr Point &operator+=(Point &lhs, const Dist &rhs) {
    lhs.x += rhs.w;
    lhs.y += rhs.h;
    return lhs;
  }

  /// A rectangular area, defined by its absolute offset in x and y direction,
  /// and its respective extents.
  struct Rect : Point, Dist {};

  constexpr coord right(const Rect &area) { return area.x + area.w; }
  constexpr coord bottom(const Rect &area) { return area.y + area.h; }

  /// 1bpp b/w, vertical tiles (1 bytes contains 8 rows)
  template <Dist extents> class Binary_frame_buffer {
    public:
      static_assert((extents.w >= 0) && (extents.h >= 0));

      static constexpr auto tile_size = 8;
      static constexpr auto buffer_size = (extents.w * extents.h) / tile_size;

      constexpr explicit Binary_frame_buffer(
          Array<u8, buffer_size> &buffer)
          : buffer_{buffer} {}

      /// Fills the entire buffer with the foreground color.
      /// Cursor is not modified.
      Binary_frame_buffer &fill_all() {
        for (auto &tile : buffer_) {
          tile = ones(tile_size);
        }
        return *this;
      }

      /// Fills the entire buffer with the background color.
      /// Cursor is not modified.
      Binary_frame_buffer &clear_all() {
        for (auto &tile : buffer_) {
          tile = 0;
        }
        return *this;
      }

      void set(const int x, const int y) {
        if ((x >= 0) and (x < width_) and (y >= 0) and (y < height_)) {
          buffer_[x + ((y / 8) * width_)] |= 1_b << (y % 8);
        }
      }

      void clear(const int x, const int y) {
        if ((x >= 0) and (x < width_) and (y >= 0) and (y < height_)) {
          buffer_[x + ((y / 8) * width_)] &= static_cast<uint8_t>(
              ~(1U << (y % 8)));
        }
      }

      void line(const int start_x, const int start_y, const int end_x,
                const int end_y) {
        // Bresenhem's algorithm
        auto delta_x = std::abs(end_x - start_x);
        auto delta_y = std::abs(end_y - start_y);
        auto sign_x = (start_x < end_x) ? 1 : -1;
        auto sign_y = (start_y < end_y) ? 1 : -1;
        auto error = delta_x - delta_y;

        auto x = start_x;
        auto y = start_y;

        set(end_x, end_y);
        while ((x != end_x) or (y != end_y)) {
          set(x, y);
          const auto double_error = error * 2;
          if (double_error > -delta_y) {
            error -= delta_y;
            x += sign_x;
          }

          if (double_error < delta_x) {
            error += delta_x;
            y += sign_y;
          }
        }
      }

      void bitmap(const std::byte *bitmap, int x_offset, int y_offset,
                  int width, int height);

      template <class Display_driver_>
      bool update(Display_driver_ &driver) const {
        static_assert((width_ <= Display_driver_::max_width)
                      && (height_ <= Display_driver_::max_height));

        for (auto page = 0; page < (height_ / 8); ++page) {
          if (!(driver.set_column_address(0) && driver.set_page_address(page)
                && driver.send_data(buffer_.data() + (page * width_),
                                    width_))) {
            return false;
          }
        }
        return true;
      }

    private:
      Array<std::byte, buffer_size> &buffer_{};
  };

  class Screen {
    public:
      static constexpr auto width = 128;
      static constexpr auto height = 64;

      /// the number of characters that one tab should entail
      static constexpr auto tab_size = 8;

      struct Character {
          char32_t character;
          const std::byte *bitmap;
      };

      struct Font {
          int width;
          int height;
          Size_type number_of_characters;
          const Character *characters;
          const Character &tofu;
      };

      explicit Screen(Display_buffer<width, height> &display_buffer)
          : buffer_{display_buffer} {}

      Screen &fill() {
        buffer_.fill_all();
        return *this;
      }

      Screen &clear() {
        buffer_.clear_all();
        return *this;
      }

      Screen &set_font(const Font &font) {
        font_ = &font;
        return *this;
      }

      Screen &at(const int x, const int y) {
        x_ = x;
        y_ = y;
        return *this;
      }

      Screen &move(const int delta_x, const int delta_y) {
        x_ += delta_x;
        y_ += delta_y;
        return *this;
      }

      Screen &draw_pixel() {
        buffer_.set(x_, y_);
        return *this;
      }

      Screen &line_to(const int x, const int y) {
        buffer_.line(x_, y_, x, y);
        return at(x, y); // move cursor to end-point
      }

      Screen &hline_to(const int x) {
        buffer_.line(x_, y_, x, y_);
        return at(x, y_);
      }

      Screen &vline_to(const int y) {
        buffer_.line(x_, y_, x_, y);
        return at(x_, y);
      }

      template <typename... Args_> Screen &print(Args_... args) {
        ent::print(print_buffer_, args...);
        print_(data(print_buffer_));
        return *this;
      }

    private:
      static const std::byte *get_character(const Font &font, char32_t c);

      void print_(const Char_type *str);
      void print_(char32_t c);

      Display_buffer<width, height> &buffer_;
      std::array<Char_type, 64U> print_buffer_{};
      int x_{0};
      int y_{0};
      const Font *font_{nullptr};
  };

} // namespace meter

#endif // LUFTENTFEUCHTER_DISPLAY_HPP_
