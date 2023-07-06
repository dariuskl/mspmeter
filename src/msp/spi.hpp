// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#ifndef MSPMETER_SPI_HPP
#define MSPMETER_SPI_HPP

#include "msp430.hpp"
#include "msp430i2.hpp"

#include <future.hpp>

namespace msp430 {

  class SPI {
    public:
      void configure() {
        using namespace msp430i2;

        UCB0::enable_reset();
        const auto ctlw0 = b_or<u16>(UCCKPH::ChangeFirst_CaptureSecond,
                                     UCCKPL::InactiveHigh, UCMSB::MSBFirst,
                                     UCMST::Master, UCMODE::SPI_4pin_nSSEL,
                                     UCSSEL::ACLK, UCSTEM::SlaveSelect);
        // P1.4 is UCB0STE, P1.5 is UCB0CLK, P1.7 is UCB0SIMO
        set_bits(PASEL0, PA::P1_4 | PA::P1_5 | PA::P1_7);
        UCB0::set_control(ctlw0);
        UCB0::set_interrupts(UCTXIE);
        transmit_next();
      }

      /// Initiates a transmission by putting the first byte into the output
      /// buffer.
      bool transmit(const u8 *const data, const Size data_len) {
        if ((data_len_ == 0) && (data_len > 0)) {
          data_ = data + 1;
          data_len_ = data_len;
          msp430i2::UCB0::write_tx_buffer(*data);
          return true;
        }
        return false;
      }

      bool transmit_next() {
        if (data_len_ > 1) {
          const auto next_byte = *data_;
          data_ += 1;
          data_len_ -= 1;
          msp430i2::UCB0::write_tx_buffer(next_byte);
          return true;
        }
        data_len_ = 0;
        return false;
      }

    private:
      const u8 *volatile data_{nullptr};
      volatile Size data_len_{0};
  };

} // namespace msp430

#endif // MSPMETER_SPI_HPP
