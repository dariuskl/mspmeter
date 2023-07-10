// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#ifndef MSPMETER_READOUT_HPP
#define MSPMETER_READOUT_HPP

#include "future.hpp"
#include "msp/spi.hpp"
#include "util.hpp"

namespace meter {

  class Readout {
    public:
      void update(const Array<char, 6> &upper, const Array<char, 6> &lower) {
        to_7segment(slice<0, 4>(display_buffer_), upper);
        to_7segment(slice<4, 4>(display_buffer_), lower);
        spi_.transmit(display_buffer_.data(), display_buffer_.size());
      }

      void on_tx_buffer_empty() { spi_.transmit_next(); }

    private:
      Array<u8, 8> display_buffer_{};
      msp430::SPI<msp430i2::UCB0> spi_{};
  };

} // namespace meter

#endif // MSPMETER_READOUT_HPP
