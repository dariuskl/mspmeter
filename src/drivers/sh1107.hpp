// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#ifndef MSPMETER_DRIVERS_SH1107_HPP_
#define MSPMETER_DRIVERS_SH1107_HPP_

#include "util/safeint.hpp"

#include <cstddef>
#include <cstdint>

namespace meter {

  constexpr auto SH1107_i2c_address = 0x3c_b;

  template <class Serial_interface_driver_type> class SH1107 {
    public:
      using coordinate = int_fast16_t;
      using extent = int_fast16_t;

      static constexpr auto max_width = extent{128};
      static constexpr auto max_height = extent{128};

      SH1107(Serial_interface_driver_type &serial) : serial_{serial} {}

      bool init() {
        return turn_display_on(false) && set_segment_remap(true)
               && set_common_output_scan_direction(true)
               && set_multiplex_ratio(96) && set_dcdc_settings(0_b, false)
               && set_display_clock_divider_oscillator_frequency(1, 0b1111_b)
               && set_precharge_discharge_periods(0_b, 2_b)
               && set_vcom_deselect_level(0x40_b);
      }

      bool send_data(const std::byte *const data, const int data_length) {
        return write_data_(data, data_length);
      }

      // commands

      bool set_column_address(const int column_address) {
        const auto cad = static_cast<std::byte>(column_address);
        return write_command_(0b0000'0000_b | (cad & 0b1111_b))
               && write_command_(0b0001'0000_b | ((cad >> 4U) & 0b111_b));
      }

      /// 1 - vertical, 0 - page (default)
      bool set_memory_addressing_mode(const std::byte mode) {
        return write_command_(0b0010'0000_b | (mode & 1_b));
      }

      bool set_contrast(const std::byte contrast) {
        return write_command_(0b1000'0001_b) && write_command_(contrast);
      }

      /// mirror vertically
      bool set_segment_remap(const bool flip) {
        return write_command_(0b1010'0000_b | (flip ? 1_b : 0_b));
      }

      bool set_multiplex_ratio(const int multiplex_ratio) {
        return write_command_(0b1010'1000_b)
               && write_command_(static_cast<std::byte>(multiplex_ratio - 1)
                                 & 0b0111'1111_b);
      }

      /// all pixels lit; RAM contents are ignored
      bool all_on(bool const d = true) {
        return write_command_(0b1010'0100_b | (d ? 1_b : 0_b));
      }

      /// invert "color"
      bool set_normal_reverse_display(const bool invert) {
        return write_command_(0b1010'0110_b | (invert ? 1_b : 0_b));
      }

      bool set_display_offset(const std::byte offset) {
        return write_command_(0xd3_b) && write_command_(offset & 0x7f_b);
      }

      bool set_dcdc_settings(const std::byte switch_frequency,
                             const bool internal_dcdc_enabled) {
        return write_command_(0b1010'1101_b)
               && write_command_(0b1000'0000_b
                                 | ((switch_frequency & 0b111_b) << 1U)
                                 | (internal_dcdc_enabled ? 1_b : 0_b));
      }

      bool turn_display_on(bool const d = true) {
        return write_command_(0b1010'1110_b | (d ? 1_b : 0_b));
      }

      bool set_page_address(const int page_address) {
        return write_command_(
            0b1011'0000_b | (static_cast<std::byte>(page_address) & 0b1111_b));
      }

      /// mirror horizontally
      bool set_common_output_scan_direction(const bool reverse) {
        return write_command_(0b1100'0000_b | (reverse ? 0b1000_b : 0b0000_b));
      }

      bool set_display_clock_divider_oscillator_frequency(
          const int clock_divider, const std::byte oscillator_frequency) {
        return write_command_(0b1101'0101_b)
               && write_command_(
                   (static_cast<std::byte>(clock_divider - 1) & 0b1111_b)
                   | ((oscillator_frequency & 0b1111_b) << 4U));
      }

      bool set_precharge_discharge_periods(const std::byte precharge,
                                           const std::byte discharge) {
        return write_command_(0b1101'1001_b)
               && write_command_((precharge & 0b1111_b)
                                 | ((discharge & 0b1111_b) << 4U));
      }

      bool set_vcom_deselect_level(const std::byte level) {
        return write_command_(0b1101'1011_b) && write_command_(level);
      }

      bool set_display_start_line(const std::byte offset) {
        return write_command_(0xdc_b) && write_command_(offset & 0x7f_b);
      }

    private:
      [[nodiscard]] bool write_command_(const std::byte command) {
        if (serial_.transmit(0_b, &command, 1)) {
          while (serial_.busy()) {
          }
          return true;
        }
        return false;
      }

      [[nodiscard]] bool write_data_(const std::byte *buffer,
                                     int buffer_length) {
        return serial_.transmit(0x40_b, buffer, buffer_length);
      }

      Serial_interface_driver_type &serial_;
  };

} // namespace meter

#endif // LUFTENTFEUCHTER_DRIVERS_SH1107_HPP_
