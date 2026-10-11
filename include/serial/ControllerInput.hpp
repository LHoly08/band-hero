#pragma once

#include <algorithm>
#include <cstdint>

namespace bh {

// Packets contain a complete state, not a delta. Sections hold encoded values
// (such as fret numbers); other instruments use independent button bits.
class ControllerState {
public:
  std::uint32_t update(std::uint32_t state, unsigned sections = 0,
                       unsigned bitsPerSection = 0) noexcept {
    state &= 0x3fffffff;
    std::uint32_t presses{};
    if (sections && bitsPerSection && bitsPerSection <= 30) {
      const auto mask = (1u << bitsPerSection) - 1;
      sections = std::min(sections, 30 / bitsPerSection);
      for (unsigned i = 0; i < sections; ++i) {
        const auto shift = i * bitsPerSection;
        const auto value = (state >> shift) & mask;
        if (value && value != ((m_state >> shift) & mask)) {
          presses |= value << shift;
        }
      }
    } else {
      presses = state & ~m_state;
    }
    m_state = state;
    return presses;
  }

private:
  std::uint32_t m_state{};
};

} // namespace bh
