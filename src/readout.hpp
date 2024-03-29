// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#ifndef MSPMETER_READOUT_HPP
#define MSPMETER_READOUT_HPP

#include "future.hpp"
#include "msp/spi.hpp"
#include "util.hpp"

namespace meter {

  class Readout {
    public:
      bool idle() const { return idle_; }

      void update(const Array<char, 6> &upper, const Array<char, 6> &lower) {
        to_7segment(slice<0, 4>(display_buffer_), upper);
        to_7segment(slice<4, 4>(display_buffer_), lower);
        idle_ = !spi_.transmit(display_buffer_.data(), display_buffer_.size());
      }

      void on_tx_buffer_empty() { idle_ = !spi_.transmit_next(); }

    private:
      Array<u8, 8> display_buffer_{};
      msp430::SPI<msp430i2::UCB0> spi_{};

      volatile bool idle_{true};
  };

} // namespace meter

#endif // MSPMETER_READOUT_HPP
