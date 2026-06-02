#pragma once
#include <array>
#include <bitset>

#include "instruments/Instrument.hpp"

namespace bh {

template <Difficulty Dif, unsigned char NumberStrings> class Bass;

/* Easy Bass */

template <unsigned char NumberStrings>
class Bass<Difficulty::EASY, NumberStrings> : Instrument {
public:
  inline constexpr unsigned char getNumberStrings() const noexcept {
    return NumberStrings;
  }
  bool getPlay(const unsigned char index,
               const unsigned char stringVal) const noexcept;

private:
  std::bitset<NumberStrings> m_stringsPlay{};
};

/* Hard Bass */

template <unsigned char NumberStrings>
class Bass<Difficulty::HARD, NumberStrings> : Instrument {
public:
  inline constexpr unsigned char getNumberStrings() const noexcept {
    return NumberStrings;
  }
  bool getPlay(const unsigned char index,
               const unsigned char fretVal) const noexcept;

private:
  std::array<unsigned char, NumberStrings> m_fretsRequired{};
};

} // namespace bh
