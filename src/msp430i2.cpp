// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#include "msp430i2.hpp"

#include "future.hpp"
#include "msp430.hpp"

namespace msp430i2 {

  namespace {

    [[gnu::section(".device_descriptor")]] Device_descriptor_and_checksum
        device_descriptor_;

  } // namespace

  bool calibrate_peripherals() {
    if (const auto ifg = load(SFR_IFG1); (ifg & BORIFG) != u8{0U}) {
      const auto calculated_checksum = calculate_device_descriptor_checksum(
          device_descriptor_.words);
      if (calculated_checksum != device_descriptor_.checksum) {
        return false;
      }
      store(REFCAL1, device_descriptor_.fields.ref_calibration.refcal1);
      store(REFCAL0, device_descriptor_.fields.ref_calibration.refcal0);
      store(CSIRFCAL, device_descriptor_.fields.dco_calibration.csirfcal);
      store(CSIRTCAL, device_descriptor_.fields.dco_calibration.csirtcal);
      store(SD24TRIM, device_descriptor_.fields.sd24_calibration.trim);
      store(SFR_IFG1, ifg & ~BORIFG);
    }
    return true;
  }

  extern "C" [[noreturn, gnu::naked]] void on_reset() {
    // init stack pointer
    extern const uint16_t _stack;
    asm volatile("mov %0, SP" : : "ri"(&_stack));

    // .data
    extern uint16_t _sdata[];        // .data start
    extern uint16_t _edata;          // .data end
    extern const uint16_t _sidata[]; // .data init values
    const auto data_count = &_edata - &(_sdata[0]);
    for (auto i = 0; i < data_count; ++i) {
      _sdata[i] = _sidata[i];
    }

    // .bss
    extern uint16_t _sbss[]; // .bss start
    extern uint16_t _ebss;   // .bss end
    const auto bss_count = &_ebss - &(_sbss[0]);
    for (auto i = 0; i < bss_count; ++i) {
      _sbss[i] = 0U;
    }

    // .preinit_array
    extern void (*_preinit_array_start[])();
    extern void (*_preinit_array_end[])();
    const auto num_preinits = &(_preinit_array_end[0])
                              - &(_preinit_array_start[0]);
    for (auto i = 0; i < num_preinits; ++i) {
      _preinit_array_start[i]();
    }

    // .init_array
    extern void (*_init_array_start[])();
    extern void (*_init_array_end[])();
    const auto num_inits = &(_init_array_end[0]) - &(_init_array_start[0]);
    for (auto i = 0; i < num_inits; ++i) {
      _init_array_start[i]();
    }

    asm volatile("call #main");
  }

} // namespace msp430i2
