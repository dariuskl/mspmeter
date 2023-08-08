// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#ifndef MSPMETER_CONFIG_HPP
#define MSPMETER_CONFIG_HPP

#include "calibration.hpp"
#include "msp430i2.hpp"

namespace meter {

  constexpr auto used_channels = 4;

  /// The number of samples, which are accumulated in the background and
  /// averaged to obtain one "reading" that is displayed to the user.
  constexpr auto number_of_oversamples = 256;

  constexpr auto default_calibration =
      Array<Channel_calibration, used_channels>{
          {{5'000'000, 30'000'000, msp430i2::SD24::full_scale, 0},
           {1'000'000, 1'000'000, msp430i2::SD24::full_scale, 0},
           {5'000'000, 30'000'000, msp430i2::SD24::full_scale, 0},
           {1'000'000, 1'000'000, msp430i2::SD24::full_scale, 0}}};

  constexpr auto heartbeat_pin = msp430i2::PA::P1_0;
  constexpr auto rx_pin = msp430i2::PA::P1_2;
  constexpr auto tx_pin = msp430i2::PA::P1_3;
  constexpr auto rclk_pin = msp430i2::PA::P1_4;
  constexpr auto srclk_pin = msp430i2::PA::P1_5;
  constexpr auto buzzer_pin = msp430i2::PA::P1_6;
  constexpr auto ser_pin = msp430i2::PA::P1_7;
  constexpr auto ns1_pressed_pin = msp430i2::PA::P2_0;
  constexpr auto encoder_a_pin = msp430i2::PA::P2_1;
  constexpr auto encoder_b_pin = msp430i2::PA::P2_2;

} // namespace meter

#endif // MSPMETER_CONFIG_HPP
