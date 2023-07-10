// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#ifndef MSPMETER_SPI_HPP
#define MSPMETER_SPI_HPP

#include "msp430.hpp"
#include "msp430i2.hpp"

#include <future.hpp>

namespace msp430 {

  template <class Peripheral_> class SPI {
    public:
      static void configure() {
        using namespace msp430i2;

        Peripheral_::enable_reset();
        const auto ctlw0 = b_or<u16>(UCCKPH::ChangeFirst_CaptureSecond,
                                     UCCKPL::InactiveHigh, UCMSB::MSBFirst,
                                     UCMST::Master, UCMODE::SPI_4pin_nSSEL,
                                     UCSSEL::ACLK, UCSTEM::SlaveSelect);
        Peripheral_::set_control(ctlw0);
        Peripheral_::set_interrupts(UCTXIE);
      }

      /// Initiates a transmission by putting the first byte into the output
      /// buffer.
      bool transmit(const u8 *const data, const Size data_len) {
        if ((data_len_ == 0) && (data_len > 0)) {
          data_ = data + 1;
          data_len_ = data_len;
          Peripheral_::write_tx_buffer(*data);
          return true;
        }
        return false;
      }

      bool transmit_next() {
        if (data_len_ > 1) {
          const auto next_byte = *data_;
          data_ += 1;
          data_len_ -= 1;
          Peripheral_::write_tx_buffer(next_byte);
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
