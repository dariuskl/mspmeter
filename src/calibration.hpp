// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#ifndef MSPMETER_CALIBRATION_HPP
#define MSPMETER_CALIBRATION_HPP

#include <cstdint>

namespace meter {

  struct Channel_calibration {
      /// The voltage that is used to calibrate the channel.
      int32_t calibration_voltage;
      /// The voltage on the attenuator inputs corresponding to
      /// `full_scale_reading`.
      int32_t full_scale_voltage;
      /// The conversion result for a full scale voltage applied at the inputs
      /// of the attenuator network.
      int32_t full_scale_reading;
      /// The conversion result when shorting the inputs of the attenuator
      /// network.
      int32_t offset;
  };

} // namespace meter

#endif // MSPMETER_CALIBRATION_HPP
