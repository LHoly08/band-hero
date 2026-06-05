#pragma once

#include <bitset>

#include "instruments/Instrument.hpp"

namespace bh {

template <Difficulty Dif> struct DrumsNums;

template <> struct DrumsNums<Difficulty::EASY> {
  enum {
    Pedals = 1,
    Pads = 3,
  };
};

template <> struct DrumsNums<Difficulty::HARD> {
  enum {
    Pedals = 2,
    Pads = 6,
  };
};

template <Difficulty Dif> class Drums : public Instrument {
public:
  inline bool getPlay(const std::uint32_t &pedalsPads) const noexcept {
    return !(
        (m_pads.to_ulong() | (m_pedals.to_ulong() << DrumsNums<Dif>::Pads)) ^
        pedalsPads);
  }

private:
  std::bitset<DrumsNums<Dif>::Pedals> m_pedals{};
  std::bitset<DrumsNums<Dif>::Pads> m_pads{};
};

} // namespace bh
