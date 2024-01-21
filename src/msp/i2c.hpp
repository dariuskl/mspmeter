// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#ifndef MSPMETER_I2C_HPP
#define MSPMETER_I2C_HPP

#include "msp430i2.hpp"
#include "util/safeint.hpp"

namespace msp430 {

  /** Driver that allows static configuration of eUSCI_B peripheral for
   *  I²C master operation and allows communication with a single slave per
   *  instance.
   * \note External pull-up resistors to a voltage <= Vcc are required.
   * \note There are no synchronization mechanism for multiple instances
   *  in place.
   */
  template <class Peripheral_, std::byte slave_address> class I2C_master {
    public:
      static void configure() {
        using namespace msp430i2;

        Peripheral_::enable_reset();
        const auto ctlw0 = b_or<u16>(UCA10::OwnAddressIs7Bit,
                                     UCSLA10::SlaveAddressIs7Bit,
                                     UCMM::SingleMaster, UCMST::Master,
                                     UCMODE::I2C, UCSYNC, UCSSEL::ACLK);
        Peripheral_::set_control(ctlw0);
        Peripheral_::set_interrupts(UCTXIE);
      }

      bool transmit(const std::byte *const data, const int data_len) {
        if ((data_len_ == 0) && (data_len > 0)) {
          data_ = data;
          data_len_ = data_len;
          sub_address_size_ = 0;
          Peripheral_::start_i2c_transmission(slave_address);
          return true;
        }
        return false;
      }

      bool transmit(const std::byte sub_address, const std::byte *const data,
                    const int data_len) {
        if ((data_len_ == 0) && (data_len > 0)) {
          data_ = data;
          data_len_ = data_len;
          sub_address_ = sub_address;
          sub_address_size_ = 1;
          Peripheral_::start_i2c_transmission(slave_address);
          return true;
        }
        return false;
      }

      bool busy() const {
        return Peripheral_::is_i2c_bus_busy();
      }

      bool transmit_next() {
        if (data_len_ > 1) {
          Peripheral_::write_tx_buffer(next());
          return true;
        }
        Peripheral_::generate_i2c_stop();
        data_len_ = 0;
        return false;
      }

    private:
      std::byte next() {
        if (sub_address_size_ > 0) {
          sub_address_size_ -= 1;
          return sub_address_;
        } else {
          const auto next_byte = *data_;
          data_ += 1;
          data_len_ -= 1;
          return next_byte;
        }
      }

      const std::byte *volatile data_{nullptr};
      volatile int data_len_{0};
      volatile std::byte sub_address_{0U};
      volatile int sub_address_size_{0};
  };

} // namespace msp430

#endif // MSPMETER_I2C_HPP
