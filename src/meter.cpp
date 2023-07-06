// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#include "meter.hpp"

#include <cstring>

namespace meter {

  constexpr auto max_channels = 4;
  static_assert(used_channels <= max_channels);

  namespace {

    constexpr auto menu_items_ = Array<std::string_view, 5>{
        {" . . . ", "1.OFF", "1.FUL", "2.OFF", "2.FUL"}};

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
    auto serial = UART{};

    Array<char, 6> upper_text_buffer_{"00.00"};
    Array<char, 6> lower_text_buffer_{"0.000"};

    auto tx_buffer = Array<char, 64>{};

  } // namespace

  [[noreturn]] void error([[maybe_unused]] const Error_code code) {
    msp430::Watchdog_timer::hold();
    msp430::disable_interrupts();
    clear_bits(msp430i2::PAOUT, msp430i2::PA::P1_0);
    while (true) {
      const auto numeric_code = std::to_underlying(code) + 1;
      for (auto i = 0; i < numeric_code; ++i) {
        set_bits(msp430i2::PAOUT, msp430i2::PA::P1_0);
        msp430i2::delay_for(250);
        clear_bits(msp430i2::PAOUT, msp430i2::PA::P1_0);
        msp430i2::delay_for(250);
      }
      msp430i2::delay_for(1'000);
    }
  }

  void Meter::init() {
    // TODO choose a timeout that is slightly above the SD24 conversion time of
    //  250 µs * 256 samples averaged in software = 64 ms
#if 0
    msp430::Watchdog_timer::configure(msp430::Watchdog_timer_clock_source::ACLK,
                                      msp430::Watchdog_timer_interval::By8192);
#endif

    constexpr auto inputs = msp430i2::PA::P2_0 | msp430i2::PA::P2_1
                            | msp430i2::PA::P2_2;

    store(msp430i2::PADIR, msp430i2::PA::All & (~inputs));
    set_bits(msp430i2::PAOUT, msp430i2::PA::P1_0 | msp430i2::PA::P2_3);
    set_bits(msp430i2::PAIES,
             msp430i2::PA::P2_0
                 // set next edge depending on current pin state for encoder
                 | (load(msp430i2::PAIN)
                    & (msp430i2::PA::P2_1 | msp430i2::PA::P2_2)));
    set_bits(msp430i2::PAIE,
             msp430i2::PA::P2_0 | msp430i2::PA::P2_1 | msp430i2::PA::P2_2);

    update_encoder();

    UART::configure<msp430i2::PA::P1_2, msp430i2::PA::P1_3>();
    AD_converter::configure();
    readout_.configure();

    msp430::enable_interrupts();

    AD_converter::start_conversion();
  }

  void Meter::step() {
    set_bits(msp430i2::PAOUT, msp430i2::PA::P1_0);

    if (AD_converter::overflow()) {
      error(Error_code::ConversionOverflow);
    }

    for (auto i = 0; i < used_channels; ++i) {
      conversion_results_[i] = converter.get_conversion_result(i);
    }

    switch (std::exchange(command_, -1)) {
    case 0:
      menu_active_ = false;
      break;
    case 1:
      calibration_.channel[0].offset = static_cast<int16_t>(
          conversion_results_[0]);
      break;
    case 2:
      calibration_.channel[0].full_scale_reading =
          conversion_results_[0] - calibration_.channel[0].offset;
      break;
    case 3:
      calibration_.channel[1].offset = static_cast<int16_t>(
          conversion_results_[1]);
      break;
    case 4:
      calibration_.channel[1].full_scale_reading =
          conversion_results_[0] - calibration_.channel[0].offset;
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
                   upper_text_buffer_.size());

      switch (count_ / 2) {
      case 0:
        lower_text_buffer_.fill('\0');
        break;
      case 1:
      case 2:
        format_readout<2, 2>(lower_text_buffer_,
                             static_cast<int16_t>(voltages_uV_[0] / 1000));
        break;
      case 3:
      case 4:
        format_readout<1, 3>(lower_text_buffer_,
                             static_cast<int16_t>(voltages_uV_[1] / 1000));
        break;
      }
    } else {
      format_readout<2, 2>(upper_text_buffer_,
                           static_cast<int16_t>(voltages_uV_[0] / 1000));

      format_readout<1, 3>(lower_text_buffer_,
                           static_cast<int16_t>(voltages_uV_[1] / 1000));
    }

    readout_.set_readings(upper_text_buffer_, lower_text_buffer_);

    // FIXME send what is being displayed
    if (const auto num_chars = print(tx_buffer.begin(), tx_buffer.size(),
                                     voltages_uV_[0], "\t", voltages_uV_[1],
                                     "\r\n");
        num_chars > 0) {
      if (!serial.transmit(tx_buffer.begin(), num_chars)) {
        error(Error_code::SerialBusy);
      }
    } else {
      error(Error_code::StringConversionFailure);
    }

    clear_bits(msp430i2::PAOUT, msp430i2::PA::P1_0);
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

  bool Meter::eusci_b0_tx_buffer_empty_isr() {
    readout_.on_tx_buffer_empty();
    return false;
  }

  bool Meter::sd24_1_conversion_done_isr() {
    return converter.on_conversion_done();
  }

  bool Meter::on_s1_down() {
    if (menu_active_) {
      command_ = count_ / 2;
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
                load(msp430i2::PAIN) & (encoder_a_pin | encoder_b_pin))),
        0, (menu_items_.size() - 1) * 2);
  }

} // namespace meter
