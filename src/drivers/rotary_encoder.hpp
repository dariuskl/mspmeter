// Meter Firmware / Darius Kellermann <kellermann@proton.me>

#ifndef MSPMETER_DRIVERS_ROTARY_ENCODER_HPP
#define MSPMETER_DRIVERS_ROTARY_ENCODER_HPP

#include <utility>

namespace meter {

  /// Driver for incremental rotary encoders.
  template <typename State_type_, State_type_ a_mask, State_type_ b_mask>
  class Rotary_encoder {
    public:
      constexpr Rotary_encoder() = default;

      int on_edge(const State_type_ next_state) {
        const auto previous_state = std::exchange(state_, next_state);
        switch (previous_state) {
        case State_type_{}:
          switch (next_state) {
          case a_mask:
            return -1;
          case b_mask:
            return 1;
          }
          break;
        case a_mask:
          switch (next_state) {
          case State_type_{}:
            return 1;
          case a_mask | b_mask:
            return -1;
          }
          break;
        case b_mask:
          switch (next_state) {
          case State_type_{}:
            return -1;
          case a_mask | b_mask:
            return 1;
          }
          break;
        case a_mask | b_mask:
          switch (next_state) {
          case a_mask:
            return 1;
          case b_mask:
            return -1;
          }
          break;
        }
        return 0;
      }

    private:
      State_type_ state_{};
  };

} // namespace meter

#endif // MSPMETER_DRIVERS_ROTARY_ENCODER_HPP
