// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#include "meter.hpp"
#include "msp430i2.hpp"

namespace {

  auto meter_ = meter::Meter{};

  [[gnu::interrupt]] void default_isr() {}

  [[gnu::interrupt]] void eusci_a0_rxtx_isr() {
    switch (msp430i2::UCA0::interrupt_vector()) {
    default:
      break;
    case msp430i2::UCIVx::RxBufferFull:
      if (meter_.eusci_a0_rx_buffer_full_isr()) {
        msp430::stay_awake();
      }
      break;
    case msp430i2::UCIVx::TxBufferEmpty:
      if (meter::Meter::eusci_a0_tx_buffer_empty_isr()) {
        msp430::stay_awake();
      }
      break;
    }
  }

  [[gnu::interrupt]] void eusci_b0_rxtx_isr() {
    switch (msp430i2::UCB0::interrupt_vector()) {
    default:
      break;
    case msp430i2::UCIVx::TxBufferEmpty:
      if (meter_.eusci_b0_tx_buffer_empty_isr()) {
        msp430::stay_awake();
      }
      break;
    }
  }

  [[gnu::interrupt]] void sd24_isr() {
    switch (msp430i2::SD24::interrupt_vector()) {
    default:
      break;
    case msp430i2::SD24IVx::SD24_0:
      if (meter::Meter::sd24_1_conversion_done_isr()) {
        msp430::stay_awake();
      }
      break;
    }
  }

  [[gnu::interrupt]] void io_port_p2_isr() {
    switch (load(msp430i2::P2IV)) {
    default:
      break;
    case msp430i2::PxIV::Px_0:
      if (meter_.on_s1_down()) {
        msp430::stay_awake();
      }
      break;
    case msp430i2::PxIV::Px_1: {
      toggle_bits(msp430i2::PAIES, msp430i2::PA::P2_1);
      meter_.update_encoder();
      break;
    }
    case msp430i2::PxIV::Px_2: {
      toggle_bits(msp430i2::PAIES, msp430i2::PA::P2_2);
      meter_.update_encoder();
      break;
    }
    }
  }

  constexpr auto vtable [[gnu::used,
                          gnu::section(".vectors")]] = Array<void (*)(), 32>{
      {nullptr,           nullptr,           nullptr,     nullptr,
       nullptr,           nullptr,           nullptr,     nullptr,
       nullptr,           nullptr,           nullptr,     nullptr,
       nullptr,           nullptr,           nullptr,     nullptr,
       default_isr,       io_port_p2_isr,    default_isr, default_isr,
       default_isr,       default_isr,       default_isr, sd24_isr,
       eusci_b0_rxtx_isr, eusci_a0_rxtx_isr, default_isr, default_isr,
       default_isr,       default_isr,       default_isr, msp430i2::on_reset}};

} // namespace

int main() {
  msp430::Watchdog_timer::hold();
  if (!msp430i2::calibrate_peripherals()) {
    error(meter::Error_code::InformationMemoryIntegrity);
  }

  meter_.init();

  while (true) {
    msp430::go_to_sleep();
    meter_.step();
  }
}
