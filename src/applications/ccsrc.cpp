// Meter Firmware / Darius Kellermann <kellermann@proton.me>

/* Firmware for a constant-current source meter providing current and voltage
 * drop over load readout on a SH1107-based OLED display.
 */

#include "drivers/sh1107.hpp"
#include "msp/i2c.hpp"
#include "msp430.hpp"
#include "msp430i2.hpp"

namespace {

  constexpr auto heartbeat_pin = msp430i2::PA::P1_0;
  constexpr auto rx_pin = msp430i2::PA::P1_2;
  constexpr auto tx_pin = msp430i2::PA::P1_3;
  constexpr auto buzzer_pin = msp430i2::PA::P1_5;
  constexpr auto scl_pin = msp430i2::PA::P1_6;
  constexpr auto sda_pin = msp430i2::PA::P1_7;
  constexpr auto ns1_pressed_pin = msp430i2::PA::P2_0;
  constexpr auto encoder_a_pin = msp430i2::PA::P2_1;
  constexpr auto encoder_b_pin = msp430i2::PA::P2_2;

  auto display_port =
      msp430::I2C_master<msp430i2::UCB0, meter::SH1107_i2c_address>{};
  auto display_driver = meter::SH1107{display_port};
  auto display_buffer = meter::Draw_buffer<128, 64>{};

  void update_display() { display_driver.send_data(); }

  [[noreturn]] void run() {
    display_port.configure();
    display_driver.init();

    msp430::enable_interrupts();

    while (true) {
      update_display();
    }
  }

  [[gnu::interrupt]] void default_isr() {}

  [[gnu::interrupt]] void eusci_b0_rxtx_isr() {
    switch (msp430i2::UCB0::interrupt_vector()) {
    default:
      break;
    case msp430i2::UCIVx::TxBufferEmpty:
      display_port.transmit_next();
      break;
    }
  }

  constexpr auto vtable
      [[gnu::used, gnu::section(".vectors")]] = Array<void (*)(), 32>{
          {nullptr,           nullptr,     nullptr,     nullptr,
           nullptr,           nullptr,     nullptr,     nullptr,
           nullptr,           nullptr,     nullptr,     nullptr,
           nullptr,           nullptr,     nullptr,     nullptr,
           default_isr,       default_isr, default_isr, default_isr,
           default_isr,       default_isr, default_isr, default_isr,
           eusci_b0_rxtx_isr, default_isr, default_isr, default_isr,
           default_isr,       default_isr, default_isr, msp430i2::on_reset}};

} // namespace

int main() {
  msp430::Watchdog_timer::hold();
  if (msp430i2::calibrate_peripherals()) {
    run();
  }
}
