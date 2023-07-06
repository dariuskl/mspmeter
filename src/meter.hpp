// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#ifndef METER_HPP_
#define METER_HPP_

#include "future.hpp"
#include "msp430.hpp"
#include "msp430i2.hpp"
#include "readout.hpp"
#include "rotary_encoder.hpp"
#include "util.hpp"

namespace meter {

  constexpr auto used_channels = 4;

  /// The number of samples, which are accumulated in the background and
  /// averaged to obtain one "reading" that is displayed to the user.
  constexpr auto number_of_oversamples = 256;

  constexpr auto encoder_a_pin = msp430i2::PA::P2_1;
  constexpr auto encoder_b_pin = msp430i2::PA::P2_2;

  struct Channel_calibration {
      /// The voltage on the attenuator inputs corresponding to
      /// `full_scale_reading`.
      int32_t full_scale_voltage;
      /// The conversion result for a full scale voltage applied at the inputs
      /// of the attenuator network.
      int32_t full_scale_reading{msp430i2::SD24::full_scale};
      /// The conversion result when shorting the inputs of the attenuator
      /// network.
      int16_t offset{0};
  };

  struct Calibration_constants {
      Array<Channel_calibration, used_channels> channel;
      int32_t reference_voltage_uV{msp430i2::SD24::reference_uV};
  };

  class AD_converter {
    public:
      static void configure() {
        using namespace msp430i2;

        store(SD24CTL, SD24REFS);
        store(SD24CCTL0, SD24LSBTOG | SD24DF | SD24IE | SD24GRP);
        store(SD24CCTL1, SD24LSBTOG | SD24DF);
      }

      static void start_conversion() {
        using namespace msp430i2;

        set_bits(SD24CCTL1, SD24SC);
      }

      static bool overflow() { return msp430i2::SD24::any_overflow(); }

      constexpr int32_t get_conversion_result(const int channel) {
        return averaged_[channel];
      }

      static constexpr int32_t to_uV(const int32_t conversion_result) {
        return static_cast<int32_t>(
            (int64_t{conversion_result} * msp430i2::SD24::reference_uV)
            / msp430i2::SD24::full_scale);
      }

      /// \return Whether the MCU should wake up.
      bool on_conversion_done() {
        a0_sum_ += msp430i2::SD24::get_conversion_result(0);
        a1_sum_ += msp430i2::SD24::get_conversion_result(1);
        ++number_of_conversion_results_;

        if (number_of_conversion_results_ >= number_of_oversamples) {
          averaged_[0] = a0_sum_ / number_of_conversion_results_;
          averaged_[1] = a1_sum_ / number_of_conversion_results_;
          a0_sum_ = 0;
          a1_sum_ = 0;
          number_of_conversion_results_ = 0;
          return true;
        }
        return false;
      }

    private:
      Array<int32_t, 2> averaged_{};

      int32_t a0_sum_{};
      int32_t a1_sum_{};
      Size number_of_conversion_results_{};
  };

  class Command_parser {
    public:
      static constexpr auto commands = Array<std::string_view, 2>{
          {"READOUT.CONFIG", "CAL.CH1.OFFSET"}};

      bool add_character(const char c) {
        if (c == '\n') {
          return true;
        }
        if (index_ < state_.size()) {
          state_[index_] = c;
        }
        return false;
      }

      void evaluate() {
        for (const auto &cmd : commands) {
          if (cmd == state_.data()) {
          }
        }
      }

    private:
      Array<char, 32> state_{};
      int index_{0};
  };

  class UART {
    public:
      /// Currently fixed for a baud rate of 9600 and using ACLK.
      template <msp430i2::PA rx_pin, msp430i2::PA tx_pin>
      static void configure() {
        using namespace msp430i2;

        UCA0::enable_reset();
        const auto ctlw0 = b_or<u16>(UCSSEL::ACLK);
        store(UCA0BRW, u16{3U});
        store(UCA0MCTLW, u16{0x9200U});
        set_bits(PASEL0, rx_pin | tx_pin);
        store(UCA0CTLW0, ctlw0);
        store(UCA0IE, UCTXIE);
      }

      bool transmit(const char *const data, const Size data_len) {
        if ((data_len_ == 0) && (data_len > 0)) {
          data_ = data + 1;
          data_len_ = data_len;
          msp430i2::UCA0::write_tx_buffer(static_cast<u8>(*data));
          return true;
        }
        return false;
      }

      /// To be called from the corresponding interrupt service routine.
      bool on_tx_buffer_empty() { return transmit_next(); }

    private:
      bool transmit_next() {
        if (data_len_ > 1) {
          const auto next_char = *data_;
          data_ += 1;
          data_len_ -= 1;
          msp430i2::UCA0::write_tx_buffer(static_cast<u8>(next_char));
          return true;
        }
        data_len_ = 0;
        return false;
      }

      const char *volatile data_{nullptr};
      volatile Size data_len_{0};
  };

  enum class Error_code {
    /// The calibration constants stored in the information memory flash segment
    /// during MCU production, were found to be invalid. This is a fatal error,
    /// because measurements won't be sufficiently accurate without the
    /// calibration constants.
    InformationMemoryIntegrity,
    /// An ADC conversion result was not collected before a new one was
    /// available. This should never happen, because it is a systematic error.
    ConversionOverflow,
    /// This is a systematic error, which is caused by having a too-low serial
    /// interface speed. New measurements are present before the old ones could
    /// be transmitted.
    SerialBusy,
    /// The conversion of the measured values to string failed due to the buffer
    /// being too small to hold the converted value.
    StringConversionFailure,
  };

  /// In debug builds, this will trap execution. In release builds, the system
  /// will be reset.
  [[noreturn]] void error(Error_code code);

  class Meter {
    public:
      void init();
      void step();
      void handle_command();

      static bool eusci_a0_tx_buffer_empty_isr();
      bool eusci_a0_rx_buffer_full_isr();
      bool eusci_b0_tx_buffer_empty_isr();
      static bool sd24_1_conversion_done_isr();
      bool on_s1_down();

      void update_encoder();

    private:
      Command_parser parser_;
      Readout readout_;
      Rotary_encoder<std::underlying_type_t<decltype(encoder_a_pin)>,
                     std::to_underlying(encoder_a_pin),
                     std::to_underlying(encoder_b_pin)>
          encoder_;

      Calibration_constants calibration_{{{{30'000'000}, {1'000'000}}}};

      Array<int32_t, used_channels> conversion_results_;
      Array<int32_t, used_channels> voltages_uV_;

      bool menu_active_{false};
      int count_{0};
      int command_{-1};
  };

} // namespace meter

#endif // METER_HPP_
