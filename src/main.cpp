// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#include "meter.hpp"
#include "msp/uart.hpp"
#include "msp430i2.hpp"

namespace {

  auto upper_text_buffer_ = Array<char, 6>{"00.00"};
  auto lower_text_buffer_ = Array<char, 6>{"0.000"};

  auto meter_ = meter::Meter{upper_text_buffer_, lower_text_buffer_};
  auto readout_ = meter::Readout{};

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
      if (meter_.eusci_a0_tx_buffer_empty_isr()) {
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
      readout_.on_tx_buffer_empty();
      break;
    }
  }

  [[gnu::interrupt]] void sd24_isr() {
    switch (msp430i2::SD24::interrupt_vector()) {
    default:
      break;
    case msp430i2::SD24IVx::SD24_0:
      if (meter_.sd24_1_conversion_done_isr()) {
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
      msp430i2::Digital_io::toggle_interrupt_edge(msp430i2::PA::P2_1);
      meter_.update_encoder();
      break;
    }
    case msp430i2::PxIV::Px_2: {
      msp430i2::Digital_io::toggle_interrupt_edge(msp430i2::PA::P2_2);
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

  [[gnu::section(".calibration_data")]] const auto cal =
      meter::Calibration_constants{meter::default_calibration};

} // namespace

int main() {
  msp430::Watchdog_timer::hold();
  if (!msp430i2::calibrate_peripherals()) {
    error(meter::Meter_status::InformationMemoryIntegrity);
  }

  msp430i2::Digital_io::configure_as_output(
      meter::heartbeat_pin | meter::buzzer_pin | msp430i2::PA::P2_3);
  msp430i2::Digital_io::set(meter::heartbeat_pin | msp430i2::PA::P2_3);
  msp430i2::Digital_io::configure_interrupt(
      meter::ns1_pressed_pin
          // set next edge depending on current pin state for encoder
          | (msp430i2::Digital_io::get()
             & (meter::encoder_a_pin | meter::encoder_b_pin)),
      meter::ns1_pressed_pin | meter::encoder_a_pin | meter::encoder_b_pin);

  msp430::SPI<msp430i2::UCB0>::configure();
  msp430::UART<msp430i2::UCA0>::configure();

  msp430i2::Digital_io::select_function(meter::rx_pin          // UCA0RXD
                                            | meter::tx_pin    // UCA0TXD
                                            | meter::rclk_pin  // UCB0STE
                                            | meter::srclk_pin // UCB0CLK
                                            | meter::ser_pin,  // UCB0SIMO
                                        u8{1U});

  meter::AD_converter::init();

  meter_.update_encoder();

  msp430::enable_interrupts();

  while (true) {
    meter_.set_calibration(cal);

    auto status = meter::Meter_status::OK;

    for (auto op =
             meter::Normal_operation{upper_text_buffer_, lower_text_buffer_,
                                     meter_, readout_};
         status == meter::Meter_status::OK; status = op()) {
      msp430i2::Digital_io::clear(meter::heartbeat_pin);
      msp430::go_to_sleep();
      msp430i2::Digital_io::set(meter::heartbeat_pin);
    }

    switch (status) {
    case meter::Meter_status::InformationMemoryIntegrity:
    case meter::Meter_status::ConversionOverflow:
    case meter::Meter_status::SerialBusy:
    case meter::Meter_status::StringConversionFailure:
      meter::error(status);
      break;
    case meter::Meter_status::StoreCalibration: {
      auto fmc = msp430i2::Flash_memory_controller{msp430i2::dco_frequency_Hz};
      fmc.erase_segment(&cal);
      fmc.write(cal, meter_.cal());
      break;
    }
    case meter::Meter_status::OK:
      break;
    }
  }
}
