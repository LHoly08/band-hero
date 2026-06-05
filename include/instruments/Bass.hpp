#pragma once
#include <bitset>
#include <cstdint>

#include "instruments/Instrument.hpp"

namespace bh {

template <Difficulty Dif> class Bass;

/* Easy Bass */

template <> class Bass<Difficulty::EASY> : Instrument {
public:
  inline constexpr unsigned char getNumberStrings() const noexcept {
    return NumberStrings<Difficulty::EASY>::Bass;
  }

  inline bool getPlay(const std::bitset<NumberStrings<Difficulty::EASY>::Bass>
                          &played) const noexcept {
    return !((played ^ m_stringsPlay).any());
  }

private:
  std::bitset<NumberStrings<Difficulty::EASY>::Bass> m_stringsPlay{};
};

/* Hard Bass */

template <> class Bass<Difficulty::HARD> : Instrument {
public:
  inline constexpr unsigned char getNumberStrings() const noexcept {
    return NumberStrings<Difficulty::EASY>::Bass;
  }
  inline bool getPlay(const std::uint32_t &played) const noexcept {
    return !(played ^ m_fretsRequired);
  }

private:
  std::uint32_t m_fretsRequired{};
};

} // namespace bh
