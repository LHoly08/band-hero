#pragma once
#include <bitset>
#include <cstdint>

#include "instruments/Instrument.hpp"

namespace bh {

template <Difficulty Dif> class Guitar;

/* Easy Guitar */

template <> class Guitar<Difficulty::EASY> : Instrument {
public:
  inline constexpr unsigned char getNumberStrings() const noexcept {
    return NumberStrings<Difficulty::EASY>::Guitar;
  }
  inline bool getPlay(const std::bitset<NumberStrings<Difficulty::EASY>::Guitar>
                          &played) const noexcept {
    return !((played ^ m_stringsPlay).any());
  }

private:
  std::bitset<NumberStrings<Difficulty::EASY>::Guitar> m_stringsPlay{};
};

/* Hard Guitar */

template <> class Guitar<Difficulty::HARD> : Instrument {
public:
  inline constexpr unsigned char getNumberStrings() const noexcept {
    return NumberStrings<Difficulty::HARD>::Guitar;
  }
  inline bool getPlay(const std::uint32_t &played) const noexcept {
    return !(played ^ m_fretsRequired);
  }

private:
  std::uint32_t m_fretsRequired{};
};

} // namespace bh
