// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#ifndef MSPMETER_ADC_HPP
#define MSPMETER_ADC_HPP

#include "config.hpp"
#include "msp430i2.hpp"

namespace meter {

  class AD_converter {
    public:
      static void init() {
        using namespace msp430i2;

        store(SD24CTL, SD24REFS);
        store(SD24CCTL0, SD24LSBTOG | SD24DF | SD24IE | SD24GRP);
        store(SD24CCTL1, SD24LSBTOG | SD24DF);
      }

      static void start_conversion() { msp430i2::SD24::start_conversion(); }
      static void stop_conversion() { msp430i2::SD24::stop_conversion(); }

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

} // namespace meter

#endif // MSPMETER_ADC_HPP
