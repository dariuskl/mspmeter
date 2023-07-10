// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#ifndef MSPMETER_UART_HPP
#define MSPMETER_UART_HPP

#include "msp430i2.hpp"

namespace msp430 {

  template <class Peripheral_> class UART {
    public:
      /// Currently fixed for a baud rate of 9600 and using ACLK.
      static void configure() {
        Peripheral_::enable_reset();
        Peripheral_::set_baud_rate_control(u16{3U});
        Peripheral_::set_modulation_control(u16{0x92}, u16{0U}, false);
        Peripheral_::set_control(b_or<u16>(msp430i2::UCSSEL::ACLK));
        Peripheral_::set_interrupts(msp430i2::UCTXIE);
      }

      bool transmit(const char *const data, const Size data_len) {
        if ((data_len_ == 0) && (data_len > 0)) {
          data_ = data + 1;
          data_len_ = data_len;
          Peripheral_::write_tx_buffer(static_cast<u8>(*data));
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
          Peripheral_::write_tx_buffer(static_cast<u8>(next_char));
          return true;
        }
        data_len_ = 0;
        return false;
      }

      const char *volatile data_{nullptr};
      volatile Size data_len_{0};
  };

} // namespace msp430

#endif // MSPMETER_UART_HPP
