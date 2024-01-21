// Copyright (C) 2023 SEGULA Technologies. All rights reserved.

#include "frame_buffer.hpp"

#include <base/utf8.hpp>

namespace medref::dcu {

  namespace {

    extent calculate_line_length(const Font &font, const char8_t *str) {
      extent length = 0;
      for (; (*str != '\0') && (*str != '\n');) {
        length += get_width(font, base::next_utf8_character(&str));
      }
      return length;
    }

    auto offset_by_horizontal_alignment(const Alignment alignment,
                                        const extent max_line_length,
                                        const extent line_length) {
      switch (alignment) {
      default:
      case Alignment::TopLeft:
      case Alignment::MiddleLeft:
      case Alignment::BottomLeft:
        return 0;
      case Alignment::TopCenter:
      case Alignment::MiddleCenter:
      case Alignment::BottomCenter:
        return (max_line_length / 2) - (line_length / 2);
      case Alignment::TopRight:
      case Alignment::MiddleRight:
      case Alignment::BottomRight:
        return max_line_length - line_length;
      }
    }

    auto offset_by_vertical_alignment(const Alignment alignment,
                                      const extent max_height,
                                      const extent required_height) {
      switch (alignment) {
      default:
      case Alignment::TopLeft:
      case Alignment::TopCenter:
      case Alignment::TopRight:
        return 0;
      case Alignment::MiddleLeft:
      case Alignment::MiddleCenter:
      case Alignment::MiddleRight:
        return (max_height / 2) - (required_height / 2);
      case Alignment::BottomLeft:
      case Alignment::BottomCenter:
      case Alignment::BottomRight:
        return max_height - required_height;
      }
    }

    template <typename Tp_ = std::byte>
    constexpr Tp_ ones(const int number_of_ones) {
      const auto digits
          = std::is_enum_v<Tp_>
                ? std::numeric_limits<std::underlying_type_t<Tp_>>::digits
                : std::numeric_limits<Tp_>::digits;
      return static_cast<Tp_>(
          (1U << static_cast<unsigned>(std::clamp(number_of_ones, 0, digits)))
          - 1U);
    }

  } // namespace

  Binary_frame_buffer &Binary_frame_buffer::fill_all() {
    for (auto &tile : m_buffer) {
      tile = ones(tile_size);
    }
    return *this;
  }

  Binary_frame_buffer &Binary_frame_buffer::clear_all() {
    for (auto &tile : m_buffer) {
      tile = 0_B;
    }
    return *this;
  }

  void Binary_frame_buffer::set_(const Point &pos) {
    if ((pos.x >= 0) and (pos.x < m_extents.w) and (pos.y >= 0)
        and (pos.y < m_extents.h)) {
      const auto byte_index = pos.x + ((pos.y / 8) * m_extents.w);
      if (byte_index < ssize(m_buffer)) {
        m_buffer[byte_index] |= 1_B << (pos.y % 8);
      }
    }
  }

  void Binary_frame_buffer::clear_(const Point &pos) {
    if ((pos.x >= 0) and (pos.x < m_extents.w) and (pos.y >= 0)
        and (pos.y < m_extents.h)) {
      const auto byte_index = pos.x + ((pos.y / 8) * m_extents.w);
      if (byte_index < ssize(m_buffer)) {
        m_buffer[byte_index] &= ~(1_B << (pos.y % 8));
      }
    }
  }

  void Binary_frame_buffer::line_(const Point &start, const Point &end) {
    // Bresenhem's line drawing algorithm
    auto delta_x = std::abs(end.x - start.x);
    auto delta_y = std::abs(end.y - start.y);
    auto sign_x = (start.x < end.x) ? 1 : -1;
    auto sign_y = (start.y < end.y) ? 1 : -1;
    auto error = delta_x - delta_y;

    auto x = start.x;
    auto y = start.y;

    if (m_mode == Binary_draw_mode::Normal) {
      set_(end);
    } else {
      clear_(end);
    }

    while ((x != end.x) or (y != end.y)) {
      if (m_mode == Binary_draw_mode::Normal) {
        set_({x, y});
      } else {
        clear_({x, y});
      }

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

  void Binary_frame_buffer::fill_(const Rect &rect,
                                  const Binary_draw_mode draw_mode) {
    for (auto page = std::max(rect.y / tile_size, 0);
         page < std::min(utils::ceil_div(rect.y + rect.h, tile_size),
                         utils::ceil_div(m_extents.h, tile_size));
         ++page) {
      const auto page_offset = page * tile_size;

      const auto data = ones((rect.y + rect.h) - page_offset)
                        & ~ones(rect.y - page_offset)
                        & ones(m_extents.h - page_offset);

      for (auto column = std::max(rect.x, 0);
           column < std::min(m_extents.w, rect.x + rect.w); ++column) {
        if (const auto index = column + (page * m_extents.w);
            index < ssize(m_buffer)) {
          if (draw_mode == Binary_draw_mode::Normal) {
            m_buffer[static_cast<size_t>(index)] |= data;
          } else {
            m_buffer[static_cast<size_t>(index)] &= ~data;
          }
        }
      }
    }
  }

  void Binary_frame_buffer::bitmap(const std::byte *bitmap, const Point &pos,
                                   const Dist &extents) {
    const auto offset_in_byte = static_cast<unsigned>(
        (pos.y % 8) + ((pos.y % 8) < 0 ? 8 : 0));

    for (auto page = 0;
         page < (utils::ceil_div(extents.h, 8) + (offset_in_byte > 0U ? 1 : 0));
         ++page) {
      for (auto column = 0; column < extents.w; ++column) {
        auto data = 0_B;

        if (page < utils::ceil_div(extents.h, 8)) {
          data |= bitmap[column + (page * extents.w)] << offset_in_byte;
        }

        if ((page > 0) and (offset_in_byte > 0U)) {
          data |= bitmap[column + ((page - 1) * extents.w)]
                  >> (8 - offset_in_byte);
        }

        if (((pos.x + column) >= 0) && ((pos.x + column) < m_extents.w)
            && ((pos.y + (page * 8)) >= 0) && (page < (m_extents.h / 8))) {
          const auto index = (pos.x + column)
                             + (((pos.y + (page * 8)) / 8) * m_extents.w);
          if (index < ssize(m_buffer)) {
            if (m_mode == Binary_draw_mode::Normal) {
              m_buffer[index] |= data;
            } else {
              m_buffer[index] &= ~data;
            }
          }
        }
      }
    }
  }

  void Binary_frame_buffer::print_(const char8_t *str_to_print,
                                   const Alignment alignment,
                                   const extent max_line_length,
                                   const extent max_height) {
    if (m_font == nullptr) {
      return;
    }

    const auto anchor_x = m_pos.x;

    m_pos.x += offset_by_horizontal_alignment(
        alignment, max_line_length,
        calculate_line_length(*m_font, str_to_print));

    m_pos.y += offset_by_vertical_alignment(alignment, max_height,
                                            get_height(*m_font, str_to_print));

    for (const auto *str = str_to_print; *str != '\0';) {
      switch (const auto a_char = base::next_utf8_character(&str); a_char) {
      case U'\n':
        m_pos.y += get_line_skip(*m_font);
        m_pos.x = anchor_x
                  + offset_by_horizontal_alignment(
                      alignment, max_line_length,
                      calculate_line_length(*m_font, str));
        break;

      case U' ':
      case U' ':
        m_pos.x += m_font->width_of_a_space;
        break;

      case U' ':
        m_pos.x += m_font->width_of_a_space / 2;
        break;

      default:
        m_pos.x += print_(a_char);
        break;
      }
    }
  }

  extent Binary_frame_buffer::print_(const char32_t a_char) {
    const auto character = get_character(*m_font, a_char);
    bitmap(character.bitmap, m_pos, {character.width, m_font->height});
    return character.width;
  }

} // namespace medref::dcu
