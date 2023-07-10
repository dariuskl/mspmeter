// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#ifndef METER_HPP_
#define METER_HPP_

#include "adc.hpp"
#include "config.hpp"
#include "future.hpp"
#include "msp430.hpp"
#include "msp430i2.hpp"
#include "readout.hpp"
#include "rotary_encoder.hpp"
#include "util.hpp"

namespace meter {

  struct Channel_calibration {
      /// The voltage on the attenuator inputs corresponding to
      /// `full_scale_reading`.
      int32_t full_scale_voltage;
      /// The conversion result for a full scale voltage applied at the inputs
      /// of the attenuator network.
      int32_t full_scale_reading{msp430i2::SD24::full_scale};
      /// The conversion result when shorting the inputs of the attenuator
      /// network.
      int32_t offset{0};
  };

  struct Calibration_constants {
      Array<Channel_calibration, used_channels> channel;
      int32_t reference_voltage_uV{msp430i2::SD24::reference_uV};
  };

  enum class Command {
    None_ = -1,
    Back,
    SetCh1Offset,
    SetCh1FullScale,
    SetCh2Offset,
    SetCh2FullScale,
    Flash,
    Num_
  };

  class Command_parser {
    public:
      bool add_character(const char c) {
        if (c == '\n') {
          return true;
        }
        if (index_ < state_.size()) {
          state_[index_] = c;
        }
        return false;
      }

      void evaluate() {}

    private:
      Array<char, 32> state_{};
      int index_{0};
  };

  enum class Meter_status {
    /// The calibration constants stored in the information memory flash segment
    /// during MCU production, were found to be invalid. This is a fatal error,
    /// because measurements won't be sufficiently accurate without the
    /// calibration constants.
    InformationMemoryIntegrity = -4,
    /// An ADC conversion result was not collected before a new one was
    /// available. This should never happen, because it is a systematic error.
    ConversionOverflow = -3,
    /// This is a systematic error, which is caused by having a too-low serial
    /// interface speed. New measurements are present before the old ones could
    /// be transmitted.
    SerialBusy = -2,
    /// The conversion of the measured values to string failed due to the buffer
    /// being too small to hold the converted value.
    StringConversionFailure = -1,
    OK = 0,
    StoreCalibration = 1
  };

  /// In debug builds, this will trap execution. In release builds, the system
  /// will be reset.
  [[noreturn]] void error(Meter_status code);

  class Meter {
    public:
      constexpr Meter(Array<char, 6> &upper_text_buffer,
                      Array<char, 6> &lower_text_buffer)
          : upper_text_buffer_{upper_text_buffer},
            lower_text_buffer_{lower_text_buffer} {}

      constexpr const auto &cal() const { return calibration_; }
      void set_calibration(const Calibration_constants &cal) {
        calibration_ = cal;
      }

      Meter_status step();
      void handle_command();

      bool eusci_a0_tx_buffer_empty_isr();
      bool eusci_a0_rx_buffer_full_isr();
      bool sd24_1_conversion_done_isr();
      bool on_s1_down();

      void update_encoder();

    private:
      void format_voltage(Array<char, 6> &text_buffer);
      void format_current();

      Array<char, 6> &upper_text_buffer_;
      Array<char, 6> &lower_text_buffer_;

      Command_parser parser_{};
      Rotary_encoder<std::underlying_type_t<decltype(encoder_a_pin)>,
                     std::to_underlying(encoder_a_pin),
                     std::to_underlying(encoder_b_pin)>
          encoder_{};

      Calibration_constants calibration_{};

      Array<int32_t, used_channels> conversion_results_{};
      Array<int32_t, used_channels> voltages_uV_{};

      bool menu_active_{false};
      int count_{0};
      Command command_{Command::None_};
  };

  class Normal_operation {
    public:
      Normal_operation(Array<char, 6> &upper_text_buffer,
                       Array<char, 6> &lower_text_buffer, Meter &meter,
                       Readout &readout)
          : upper_text_buffer_{upper_text_buffer},
            lower_text_buffer_{lower_text_buffer}, meter_{meter},
            readout_{readout} {
        // TODO choose a timeout that is slightly above the SD24 conversion time
        //  of 250 µs * 256 samples averaged in software = 64 ms
#if 0
        msp430::Watchdog_timer::configure(
            msp430::Watchdog_timer_clock_source::ACLK,
            msp430::Watchdog_timer_interval::By8192);
#endif
        AD_converter::start_conversion();
      }

      ~Normal_operation() {
        meter::AD_converter::stop_conversion();
        msp430::Watchdog_timer::hold();
      }

      Meter_status operator()() {
        const auto status = meter_.step();
        if (status < Meter_status::OK) {
          print(upper_text_buffer_, "Err");
          format_readout<4, 0>(lower_text_buffer_, std::to_underlying(status));
        }
        readout_.update(upper_text_buffer_, lower_text_buffer_);
        return status;
      }

    private:
      Array<char, 6> &upper_text_buffer_;
      Array<char, 6> &lower_text_buffer_;

      Meter &meter_;
      Readout &readout_;
  };

} // namespace meter

#endif // METER_HPP_
