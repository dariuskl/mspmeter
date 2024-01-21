// Darius Kellermann <kellermann@proton.me>, October 2022

#include "draw.hpp"

namespace ent {

  template <int width_, int height_>
  void Display_buffer<width_, height_>::bitmap(std::byte const *bitmap,
                                               int const x_offset,
                                               int const y_offset,
                                               int const width,
                                               int const height) {
    const auto vertical_offset = static_cast<unsigned>(
        ((y_offset % 8) < 0) ? (8 + (y_offset % 8)) : (y_offset % 8));

    for (auto page = 0;
         page < ceil_div(height, 8) + (vertical_offset > 0U ? 1 : 0); ++page) {
      for (auto column = 0; column < width; ++column) {
        auto data = 0_b;

        if (page < ceil_div(height, 8)) {
          data |= bitmap[column + (page * width)] << vertical_offset;
        }

        if ((page > 0) and (vertical_offset > 0U)) {
          data |= bitmap[column + ((page - 1) * width)]
                  >> (8U - vertical_offset);
        }

        if (((x_offset + column) >= 0) and ((x_offset + column) < width_)
            and ((y_offset + (page * 8)) >= 0) and (page < (height_ / 8))) {
          const auto index
              = (x_offset + column) + (((y_offset + (page * 8)) / 8) * width_);
          buffer_[index] |= data;
        }
      }
    }
  }

  const std::byte *Screen::get_character(const Font &font, const char32_t c) {
    for (auto i = 0; i < font.number_of_characters; ++i) {
      if (font.characters[i].character == c) {
        return font.characters[i].bitmap;
      }
    }
    return font.tofu.bitmap;
  }

  void Screen::print_(const Char_type *str) {
    const auto anchor_x = x_;

    for (auto c : Utf8_sv{str}) {
      switch (c) {
      case 8U: // backspace
        x_ -= font_->width;
        break;

      case U'\t':
        // move to next tab stop
        x_ = ((x_ / (width / font_->width / tab_size)) + 1)
             * (font_->width * tab_size);
        break;

      case U'\n':
        y_ += font_->height + (font_->height / 2);
        break;

      case U'\r':
        x_ = anchor_x;
        break;

      case U' ':
        x_ += font_->width;
        break;

      case U' ':
        x_ += font_->width / 2;
        break;

      default:
        print_(c);
        break;
      }
    }
  }

  void Screen::print_(const char32_t c) {
    if (font_ != nullptr) {
      buffer_.bitmap(get_character(*font_, c), x_, y_, font_->width,
                     font_->height);
      x_ += font_->width;
    }
  }

} // namespace ent
