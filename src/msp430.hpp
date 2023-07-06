// Meter Firmware / Darius Kellermann <kellermann@protonmail.com>

#ifndef MSP430_HPP_
#define MSP430_HPP_

#include <cstdint>

namespace msp430 {

  enum class SR : uint16_t {
    C = 0x0001U,
    Z = 0x0002U,
    N = 0x0004U,
    GIE = 0x0008U,
    CPUOFF = 0x0010U,
    OSCOFF = 0x0020U,
    SCG0 = 0x0040U,
    SCG1 = 0x0080U,
    V = 0x01000U,
  };

  constexpr SR operator~(const SR rhs) {
    return static_cast<SR>(~std::to_underlying(rhs));
  }

  constexpr SR operator&(const SR lhs, const SR rhs) {
    return static_cast<SR>(std::to_underlying(lhs) & std::to_underlying(rhs));
  }

  constexpr SR operator|(const SR lhs, const SR rhs) {
    return static_cast<SR>(std::to_underlying(lhs) | std::to_underlying(rhs));
  }

  inline void enable_interrupts() { asm volatile("eint"); }
  inline void disable_interrupts() { asm volatile("dint { nop"); }

  class Critical_section {
    public:
      Critical_section() { disable_interrupts(); }
      ~Critical_section() { enable_interrupts(); }

      Critical_section(const Critical_section &) = delete;
      Critical_section(Critical_section &&) = default;

      Critical_section &operator=(const Critical_section &) = delete;
      Critical_section &operator=(Critical_section &&) = default;
  };

  [[gnu::always_inline]] inline void go_to_sleep() {
    asm volatile("nop { bis %0, SR { nop" : : "ri"(SR::CPUOFF));
  }

  [[gnu::always_inline]] inline void stay_awake() {
    __bic_SR_register_on_exit(std::to_underlying(SR::CPUOFF));
  }

  enum class Watchdog_timer_interval : uint16_t {
    By32768 = 0U,
    By8192 = 1U,
    By512 = 2U,
    By64 = 3U
  };
  enum class Watchdog_timer_clock_source : uint16_t { SMCLK = 0U, ACLK = 4U };

  class Watchdog_timer {
    public:
      static void configure(const Watchdog_timer_clock_source clock_source,
                            const Watchdog_timer_interval interval) {
        store(wdtctl_,
              b_or<u16>(load(wdtctl_b_), wdt_unlock_, clock_source, interval));
      }

      static void hold() { store(wdtctl_, wdt_unlock_ | wdt_hold_); }

      static void feed() {
        store(wdtctl_, wdt_unlock_ | load(wdtctl_b_) | wdt_count_clear_);
      }

    private:
      static constexpr auto wdt_unlock_ = u16{0x5a00U};
      static constexpr auto wdt_hold_ = u16{0x0080U};
      static constexpr auto wdt_count_clear_ = u16{0x0008U};
      static constexpr auto wdtctl_ = Register<u16>{0x0120};
      static constexpr auto wdtctl_b_ = Register<const u8>{0x0120};
  };

  enum class TASSEL {
    TAxCLK = 0U,
    ACLK = 1U,
    SMCLK = 2U,
    Inverted_TAxCLK = 3U
  };
  constexpr auto CCIE = u16{0x0010U};

  class Timer_TA0 {
    public:
      static void configure(const TASSEL clock_source,
                            const uint8_t clock_divider) {
        store(ta0ctl_,
              (u16{static_cast<uint16_t>(clock_source)} << 8U)
                  | (u16{static_cast<uint16_t>(clock_divider - 1)} << 6U));
      }

      u16 count() const { return load(ta0r_); }

      static void enable_interrupt() { set_bits(ta0cctl0_, CCIE); }

      void stop() { store(ta0ctl_, load(ta0ctl_) & ~(u16{3U} << 4U)); }

      static void start_up(const u16 top) {
        store(ta0ccr0_, top);
        store(ta0ctl_, (load(ta0ctl_) & ~(u16{3U} << 4U)) | (u16{1U} << 4U));
      }

      void start_continuous() {
        store(ta0ctl_, (load(ta0ctl_) & ~(u16{3U} << 4U)) | (u16{2U} << 4U));
      }

      static void start_up_down(const u16 top) {
        store(ta0ccr0_, top);
        store(ta0ctl_, (load(ta0ctl_) & ~(u16{3U} << 4U)) | (u16{3U} << 4U));
      }

    private:
      static constexpr auto ta0ctl_ = Register<u16>{0x0160};
      static constexpr auto ta0cctl0_ = Register<u16>{0x0162};
      static constexpr auto ta0r_ = Register<const u16>{0x0170};
      static constexpr auto ta0ccr0_ = Register<u16>{0x172};
  };

} // namespace msp430

#endif // MSP430_HPP_
