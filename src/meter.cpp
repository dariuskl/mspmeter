// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#include "meter.hpp"
#include "msp/uart.hpp"

#include <cstring>

namespace meter {

  constexpr auto max_channels = 4;
  static_assert(used_channels <= max_channels);

  namespace {

    constexpr auto menu_items_ =
        Array<std::string_view, std::to_underlying(Command::Num_)>{
            {"rEt", "1.OFF", "1.FUL", "2.OFF", "2.FUL", "FLSH"}};

    /// \returns the scaled voltage reading in uV
    constexpr int32_t apply(const int32_t conversion_result,
                            const Channel_calibration &cal) {
      return static_cast<int32_t>(
          (int64_t{conversion_result - cal.offset} * cal.full_scale_voltage)
          / cal.full_scale_reading);
    }
    static_assert(apply(0x7f'ffff, {30'000'000, 0x7f'ffff, 0}) == 30'000'000);
    static_assert(apply(0x6e'0000, {30'000'000, 0x6e'0000, 0}) == 30'000'000);
    static_assert(apply(0x70'0000, {30'000'000, 0x6e'0000, 0}) > 30'000'000);

    auto converter = AD_converter{};
    auto serial = msp430::UART<msp430i2::UCA0>{};

    auto tx_buffer = Array<char, 80>{};

  } // namespace

  [[noreturn]] void error(const Meter_status code) {
    msp430::Watchdog_timer::hold();
    msp430::disable_interrupts();
    msp430i2::Digital_io::clear(heartbeat_pin);
    while (true) {
      const auto numeric_code = -std::to_underlying(code);
      for (auto i = 0; i < numeric_code; ++i) {
        msp430i2::Digital_io::set(heartbeat_pin);
        msp430i2::delay_for(250);
        msp430i2::Digital_io::clear(heartbeat_pin);
        msp430i2::delay_for(250);
      }
      msp430i2::delay_for(1'000);
    }
  }

  Meter_status Meter::step() {
    if (AD_converter::overflow()) {
      return Meter_status::ConversionOverflow;
    }

    for (auto i = 0; i < used_channels; ++i) {
      conversion_results_[i] = converter.get_conversion_result(i);
    }

    switch (std::exchange(command_, Command::None_)) {
    case Command::Back:
      menu_active_ = false;
      break;
    case Command::SetCh1Offset:
      calibration_.channel[0].offset = conversion_results_[0];
      break;
    case Command::SetCh1FullScale:
      calibration_.channel[0].full_scale_reading =
          conversion_results_[0] - calibration_.channel[0].offset;
      break;
    case Command::SetCh2Offset:
      calibration_.channel[1].offset = conversion_results_[1];
      break;
    case Command::SetCh2FullScale:
      calibration_.channel[1].full_scale_reading =
          conversion_results_[1] - calibration_.channel[1].offset;
      break;
    case Command::Flash:
      menu_active_ = false;
      return Meter_status::StoreCalibration;
    case Command::None_:
    case Command::Num_:
      break;
    }

    for (auto i = 0; i < used_channels; ++i) {
      voltages_uV_[i] = apply(conversion_results_[i], calibration_.channel[i]);
    }

#if 0
    const auto &voltage_mV = voltages_uV_[0];
    const auto &current_mA = voltages_uV_[1];
    // TODO add support for switchable power display
    const auto power_mW = (voltage_mV * current_mA) / 1000;
#endif

    if (menu_active_) {
      std::strncpy(upper_text_buffer_.data(), menu_items_[count_ / 2].data(),
                   static_cast<size_t>(upper_text_buffer_.size()));

      switch (static_cast<Command>(count_ / 2)) {
      case Command::None_:
      case Command::Num_:
        break;
      case Command::Back:
      case Command::Flash:
        lower_text_buffer_.fill('\0');
        break;
      case Command::SetCh1Offset:
      case Command::SetCh1FullScale:
        format_voltage(lower_text_buffer_);
        break;
      case Command::SetCh2Offset:
      case Command::SetCh2FullScale:
        format_current();
        break;
      }
    } else {
      format_voltage(upper_text_buffer_);
      format_current();
    }

    // FIXME send what is being displayed
    if (const auto num_chars = print(
            tx_buffer, conversion_results_[0], " - ",
            calibration_.channel[0].offset, " => ", voltages_uV_[0], "\t",
            conversion_results_[1], " - ", calibration_.channel[1].offset,
            " => ", voltages_uV_[1], "\r\n");
        num_chars > 0) {
      if (!serial.transmit(tx_buffer.begin(), num_chars)) {
        return Meter_status::SerialBusy;
      }
    } else {
      return Meter_status::StringConversionFailure;
    }

    return Meter_status::OK;
  }

  void Meter::handle_command() { parser_.evaluate(); }

  bool Meter::eusci_a0_tx_buffer_empty_isr() {
    serial.on_tx_buffer_empty();
    return false;
  }

  bool Meter::eusci_a0_rx_buffer_full_isr() {
    return parser_.add_character(
        static_cast<char>(msp430i2::UCA0::read_rx_buffer()));
  }

  bool Meter::sd24_1_conversion_done_isr() {
    return converter.on_conversion_done();
  }

  bool Meter::on_s1_down() {
    if (menu_active_) {
      command_ = Command{count_ / 2};
    } else {
      menu_active_ = true;
      count_ = 0;
    }
    return false;
  }

  void Meter::update_encoder() {
    count_ = std::clamp(
        count_
            - encoder_.on_edge(std::to_underlying(
                msp430i2::Digital_io::get() & (encoder_a_pin | encoder_b_pin))),
        0, (menu_items_.size() - 1) * 2);
  }

  void Meter::format_voltage(Array<char, 6> &text_buffer) {
    if (const auto voltage_mV = voltages_uV_[0] / 1'000; voltage_mV < 10'000) {
      format_readout<1, 3>(text_buffer, saturate_cast<int16_t>(voltage_mV));
    } else {
      format_readout<2, 2>(text_buffer,
                           saturate_cast<int16_t>(voltage_mV / 10));
    }
  }

  void Meter::format_current() {
    format_readout<1, 3>(lower_text_buffer_,
                         static_cast<int16_t>(voltages_uV_[1] / 1'000));
  }

} // namespace meter
