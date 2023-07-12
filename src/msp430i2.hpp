// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#ifndef MSP430I2_HPP_
#define MSP430I2_HPP_

#include "future.hpp"

namespace msp430i2 {

  constexpr auto dco_frequency_Hz = 16'384'000;

  [[gnu::always_inline]] inline void delay_for(const int millis) {
    __delay_cycles(millis * (msp430i2::dco_frequency_Hz / 1000));
  }

  constexpr auto shared_ref_mV = 1'158;

  constexpr auto SFR_IE1 = Register<u8>{0x00};
  constexpr auto SFR_IFG1 = Register<u8>{0x02};

  constexpr auto BORIFG = u8{0x04U};

  enum class FCTL1 : uint16_t {
    FWKEY1 = 0xa500U,
    BLKWRT = 0x0080U,
    WRT = 0x0040U,
    MERAS = 0x0004U,
    ERASE = 0x0002U
  };

  constexpr FCTL1 operator|(FCTL1 lhs, FCTL1 rhs) {
    return static_cast<FCTL1>(std::to_underlying(lhs)
                              | std::to_underlying(rhs));
  }

  enum class FCTL2 : uint16_t {
    FWKEY2 = 0xa500U,
    FSSELx_MCLK = 0x0040U,
    FSSELx_SMCLK = 0x0080U,
  };

  constexpr FCTL2 operator|(FCTL2 lhs, FCTL2 rhs) {
    return static_cast<FCTL2>(std::to_underlying(lhs)
                              | std::to_underlying(rhs));
  }

  enum class FCTL3 : uint16_t {
    FWKEY3 = 0xa500U,
    LOCKSEG = 0x0040U,
    EMEX = 0x0020U,
    LOCK = 0x0010U,
    WAIT = 0x0008U,
    ACCVIFG = 0x0004U,
    KEYV = 0x0002U,
    BUSY = 0x0001U
  };

  constexpr FCTL3 operator|(FCTL3 lhs, FCTL3 rhs) {
    return static_cast<FCTL3>(std::to_underlying(lhs)
                              | std::to_underlying(rhs));
  }

  class Flash_memory_controller {
    public:
      static constexpr auto ftg_operating_frequency_Hz = 366'000;

      explicit Flash_memory_controller(const long mclk_frequency_Hz) {
        store(fctl2, FCTL2::FWKEY2 | FCTL2::FSSELx_MCLK
                         | static_cast<FCTL2>(mclk_frequency_Hz
                                              / ftg_operating_frequency_Hz));
      }

      /// Erases the segment containing the data pointed to by `ptr`.
      ///    Execution is blocked until the procedure has finished. Thus, it is
      /// advisable to hold the watchdog before calling this function.
      /// \param [in] ptr A pointer to any address within flash memory. The
      ///    segment that contains the data pointed to, will be erased.
      ///    The first segment starts at 0xffff and each segment has a size of
      ///    1 KiB.
      void erase_segment(const void *const ptr) {
        store(fctl3, FCTL3::FWKEY3);
        store(fctl1, FCTL1::FWKEY1 | FCTL1::ERASE);
        *const_cast<u16 *>(static_cast<const u16 *>(ptr)) = u16{0U};
        store(fctl1, FCTL1::FWKEY1);
        store(fctl3, FCTL3::FWKEY3 | FCTL3::LOCK);
      }

      template <typename Tp_> void write(const Tp_ &flash_obj, const Tp_ &obj) {
        store(fctl3, FCTL3::FWKEY3);
        store(fctl1, FCTL1::FWKEY1 | FCTL1::WRT);
        const_cast<Tp_ &>(flash_obj) = obj;
        store(fctl1, FCTL1::FWKEY1);
        store(fctl3, FCTL3::FWKEY3 | FCTL3::LOCK);
      }

    private:
      static constexpr auto fctl1 = Register<FCTL1>{0x0128};
      static constexpr auto fctl2 = Register<FCTL2>{0x012a};
      static constexpr auto fctl3 = Register<FCTL3>{0x012c};
  };

  enum class PA : uint16_t {
    None = 0x0000U,
    P1_0 = 0x0001U,
    P1_1 = 0x0002U,
    P1_2 = 0x0004U,
    P1_3 = 0x0008U,
    P1_4 = 0x0010U,
    P1_5 = 0x0020U,
    P1_6 = 0x0040U,
    P1_7 = 0x0080U,
    P2_0 = 0x0100U,
    P2_1 = 0x0200U,
    P2_2 = 0x0400U,
    P2_3 = 0x0800U,
    P2_4 = 0x1000U,
    P2_5 = 0x2000U,
    P2_6 = 0x4000U,
    P2_7 = 0x8000U,
    All = 0xffffU
  };

  constexpr PA operator~(const PA rhs) {
    return static_cast<PA>(~std::to_underlying(rhs));
  }

  constexpr PA operator&(const PA lhs, const PA rhs) {
    return static_cast<PA>(std::to_underlying(lhs) & std::to_underlying(rhs));
  }

  constexpr PA operator|(const PA lhs, const PA rhs) {
    return static_cast<PA>(std::to_underlying(lhs) | std::to_underlying(rhs));
  }

  constexpr PA operator^(const PA lhs, const PA rhs) {
    return static_cast<PA>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
  }

  enum class PxIV : uint16_t {
    Px_0 = 0x02U,
    Px_1 = 0x04U,
    Px_2 = 0x06U,
    Px_3 = 0x08U,
    Px_4 = 0x0aU,
    Px_5 = 0x0cU,
    Px_6 = 0x0eU,
    Px_7 = 0x10U
  };

  constexpr auto P1IV = Register<const PxIV>{0x1e};
  constexpr auto P2IV = Register<const PxIV>{0x2e};

  struct Pin_config {
      PA pin;
  };

  class Digital_io {
    public:
      static void configure_as_input(const PA pin_mask) {
        clear_bits(PADIR, pin_mask);
      }

      static void configure_interrupt(const PA edge_select,
                                      const PA interrupt_enable) {
        set_bits(PAIES, edge_select);
        set_bits(PAIE, interrupt_enable);
      }

      static void toggle_interrupt_edge(const PA pin_mask) {
        toggle_bits(PAIES, pin_mask);
      }

      static void select_function(const PA pin_mask, const u8 function) {
        switch (function) {
        case u8{0U}:
          clear_bits(PASEL0, pin_mask);
          clear_bits(PASEL1, pin_mask);
          break;
        case u8{1U}:
          set_bits(PASEL0, pin_mask);
          clear_bits(PASEL1, pin_mask);
          break;
        case u8{2U}:
          clear_bits(PASEL0, pin_mask);
          set_bits(PASEL1, pin_mask);
          break;
        case u8{3U}:
          set_bits(PASEL0, pin_mask);
          set_bits(PASEL1, pin_mask);
          break;
        }
      }

      static void set(const PA pin_mask) { set_bits(PAOUT, pin_mask); }
      static void clear(const PA pin_mask) { clear_bits(PAOUT, pin_mask); }

      static auto get() { return load(PAIN); }

    private:
      static constexpr auto PAIN = Register<const PA>{0x10};
      static constexpr auto PAOUT = Register<PA>{0x12};
      static constexpr auto PADIR = Register<PA>{0x14};
      static constexpr auto PASEL0 = Register<PA>{0x1a};
      static constexpr auto PASEL1 = Register<PA>{0x1c};
      static constexpr auto PAIES = Register<PA>{0x28};
      static constexpr auto PAIE = Register<PA>{0x2a};
  };

  enum class CSCTL : uint8_t { DCOR = 0x01U, DCOBYP = 0x02U, DCOF = 0x80U };

  constexpr auto CSCTL0 = Register<CSCTL>{0x50};
  constexpr auto CSCTL1 = Register<u8>{0x51};
  constexpr auto CSIRFCAL = Register<u8>{0x52};
  constexpr auto CSIRTCAL = Register<u8>{0x53};

  constexpr auto REFCAL0 = Register<u8>{0x62};
  constexpr auto REFCAL1 = Register<u8>{0x63};

  constexpr auto SD24REFS = u16{0x0004U};
  constexpr auto SD24OVIE = u16{0x0002U};

  constexpr auto SD24TRIM = Register<u8>{0xbf};
  constexpr auto SD24CTL = Register<u16>{0x100};

  constexpr auto UCSWRST = u16{0x0001U};

  enum class UCIVx : uint16_t {
    RxBufferFull = 2U, // highest priority
    TxBufferEmpty = 4U,
    StartBitReceived = 6U,
    TransmitComplete = 8U // lowest priority
  };

  constexpr auto UCA0CTLW0 = Register<u16>{0x0140};
  constexpr auto UCA0BRW = Register<u16>{0x0146};
  constexpr auto UCA0MCTLW = Register<u16>{0x0148};
  constexpr auto UCA0RXBUF = Register<const u16>{0x014c};
  constexpr auto UCA0TXBUF = Register<u16>{0x014e};

  constexpr auto UCTXIE = u16{0x0002U};
  constexpr auto UCTXIFG = u16{0x0002U};

  constexpr auto UCA0IE = Register<u16>{0x015a};
  constexpr auto UCA0IFG = Register<u16>{0x015c};
  constexpr auto UCA0IV = Register<UCIVx>{0x015e};

  enum class UCCKPH : uint16_t {
    ChangeFirst_CaptureSecond,
    CaptureFirst_ChangeSecond = 0x8000U
  };
  enum class UCCKPL : uint16_t { InactiveLow, InactiveHigh = 0x4000U };
  enum class UCMSB : uint16_t { LSBFirst, MSBFirst = 0x2000U };
  enum class UCMST : uint16_t { Slave, Master = 0x0800U };
  enum class UCMODE : uint16_t {
    SPI_3pin,
    SPI_4pin_SSEL = 0x0200U,
    SPI_4pin_nSSEL = 0x0400U,
    I2C = 0x0c00U
  };
  enum class UCSSEL : uint16_t { ACLK = 0x0040U, SMCLK = 0x0080U };
  enum class UCSTEM : uint16_t { MultiMaster, SlaveSelect = 0x0002U };
  constexpr auto UCBUSY = u16{1U};

  template <intptr_t base_, intptr_t ir_base_> class Eusci {
    public:
      static auto interrupt_vector() { return load(iv_); }

      static void enable_reset() { clear_bits(ctlw0_, UCSWRST); }

    private:
      static constexpr auto iv_ = Register<UCIVx>{ir_base_ + 4};
      static constexpr auto ctlw0_ = Register<u16>{base_};
  };

  class UCA0 {
    public:
      static auto interrupt_vector() { return load(UCA0IV); }

      static void enable_reset() { clear_bits(UCA0CTLW0, UCSWRST); }

      static void set_control(const u16 ctlw0) { store(UCA0CTLW0, ctlw0); }
      static void set_interrupts(const u16 ie) { store(UCA0IE, ie); }
      static void set_baud_rate_control(const u16 UCBRx) {
        store(UCA0BRW, UCBRx);
      }
      static void set_modulation_control(const u16 UCBRSx, const u16 UCBRFx,
                                         const bool oversampling_mode) {
        store(UCA0MCTLW, (UCBRSx << 8U) | (UCBRFx << 4U)
                             | (oversampling_mode ? u16{1U} : u16{0U}));
      }

      static auto read_rx_buffer() { return load(UCA0RXBUF); }
      static void write_tx_buffer(const u8 byte_to_transmit) {
        store(UCA0TXBUF, static_cast<u16>(byte_to_transmit));
      }
  };

  constexpr auto UCB0CTLW0 = Register<u16>{0x01c0};
  constexpr auto UCB0BRW = Register<u16>{0x01c6};
  constexpr auto UCB0STATW = Register<u16>{0x01c8};
  constexpr auto UCB0RXBUF = Register<u16>{0x01cc};
  constexpr auto UCB0TXBUF = Register<u16>{0x01ce};
  constexpr auto UCB0IE = Register<u16>{0x01ea};
  constexpr auto UCB0IFG = Register<u16>{0x01ec};
  constexpr auto UCB0IV = Register<UCIVx>{0x01ee};

  class UCB0 {
    public:
      static auto interrupt_vector() { return load(UCB0IV); }
      static bool busy() { return (load(UCB0STATW) & UCBUSY) != u16{0U}; }

      static void enable_reset() { clear_bits(UCB0CTLW0, UCSWRST); }

      static void set_control(const u16 ctlw0) { store(UCB0CTLW0, ctlw0); }
      static void set_interrupts(const u16 ie) { store(UCB0IE, ie); }

      static void write_tx_buffer(const u8 byte_to_transmit) {
        store(UCB0TXBUF, static_cast<u16>(byte_to_transmit));
      }
  };

  constexpr auto SD24LSBTOG = u16{0x0080U};
  constexpr auto SD24OVIFG = u16{0x0020U};
  constexpr auto SD24DF = u16{0x0010U};
  constexpr auto SD24IE = u16{0x0008U};
  constexpr auto SD24IFG = u16{0x0004U};
  constexpr auto SD24SC = u16{0x0002U};
  constexpr auto SD24GRP = u16{0x0001U};

  enum class SD24IVx : uint16_t {
    Overflow = 2U, // highest priority
    SD24_0 = 4U,
    SD24_1 = 6U,
    SD24_2 = 8U,
    SD24_3 = 10U,
    SD24_4 = 12U,
    SD24_5 = 14U,
    SD24_6 = 16U // lowest priority
  };

  constexpr auto SD24CCTL0 = Register<u16>{0x102};
  constexpr auto SD24CCTL1 = Register<u16>{0x104};
  constexpr auto SD24CCTL2 = Register<u16>{0x106};
  constexpr auto SD24CCTL3 = Register<u16>{0x108};
  constexpr auto SD24IV = Register<const SD24IVx>{0x1f0};

  constexpr auto SD24CCTLx = Array<Register<u16>, 4>{
      {{0x102}, {0x104}, {0x106}, {0x108}}};
  constexpr auto SD24MEMx = Array<Register<u16>, 4>{
      {{0x110}, {0x112}, {0x114}, {0x116}}};

  class SD24 {
    public:
      static auto interrupt_vector() { return load(SD24IV); }
      static bool any_overflow() {
        return ((load(SD24CCTL0) & SD24OVIFG) | (load(SD24CCTL1) & SD24OVIFG)
                | (load(SD24CCTL2) & SD24OVIFG) | (load(SD24CCTL3) & SD24OVIFG))
               != u16{0U};
      }

      template <int channel>
      static void start_conversion() {
        set_bits(SD24CCTLx[channel], SD24SC);
      }

      template <int channel>
      static void stop_conversion() {
        clear_bits(SD24CCTLx[channel], SD24SC);
      }

      static constexpr auto full_scale = 0x7f'ffff;
      static constexpr auto reference_uV = int32_t{msp430i2::shared_ref_mV}
                                           * 1'000;

      /// This function reads the conversion result from the register, extends
      /// it do a 32-bit integer and returns it.
      ///   The interrupt flag will be cleared by calling this method.
      static int32_t get_conversion_result(const int channel) {
        auto msb = static_cast<u32>(load(SD24MEMx[channel])) << 8U;
        if ((msb &u32{0x0080'0000U}) != u32{0U}) {
          msb = msb | u32{0xff00'0000U};
        }
        return static_cast<int32_t>(msb | load(SD24MEMx[channel]));
      }
  };

  struct [[gnu::packed]] Device_descriptor {
      struct {
          u8 tag;
          u8 length;
          u32 lot_wafer_id;
          u16 die_x_position;
          u16 die_y_position;
          u16 test_results;
      } die_record;
      struct {
          u8 tag;
          u8 length;
          u8 refcal1;
          u8 refcal0;
      } ref_calibration;
      struct {
          u8 tag;
          u8 length;
          u8 csirfcal;
          u8 csirtcal;
          u8 cserfcal;
          u8 csertcal;
      } dco_calibration;
      struct {
          u8 tag;
          u8 length;
          u8 trim;
          u8 empty;
      } sd24_calibration;
      struct {
          u8 tag_empty;
          u8 empty_length;
          Array<u8, 34> empty;
      } empty;
  };

  struct [[gnu::packed]] Device_descriptor_and_checksum {
      u16 checksum;
      union {
          Device_descriptor fields;
          Array<u16, static_cast<Size>(sizeof fields / sizeof(u16))> words;
      };
  };

  template <Size device_descriptor_size>
  constexpr u16 calculate_device_descriptor_checksum(
      const Array<u16, device_descriptor_size> &words) {
    auto checksum = u16{};
    for (auto word : words) {
      checksum = checksum ^ word;
    }
    return twos_complement(checksum);
  }

  bool calibrate_peripherals();
  extern "C" [[noreturn]] void on_reset();

} // namespace msp430i2

#endif // MSP430I2_HPP_
