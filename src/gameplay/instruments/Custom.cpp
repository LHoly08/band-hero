#include "gameplay/instruments/Custom.hpp"

#include "gameplay/instruments/Instrument.hpp"

namespace bh {

template <>
bool Custom<InstrumentType::Custom_1, Difficulty::Easy>::getPlay(
    std::uint32_t playedNote) noexcept {
  // Easy accepts any input within a section; Hard compares its complete value.
  // Both discard bits outside the composition.

  playedNote &=
      ((1 << (m_composition.NumberSections * m_composition.NumberBitsSection)) -
       1);

  return Base::getPlay(playedNote,
                       {m_composition.NumberSections,
                        m_composition.NumberBitsSection});
}

template <>
bool Custom<InstrumentType::Custom_1, Difficulty::Hard>::getPlay(
    std::uint32_t playedNote) noexcept {

  playedNote &=
      ((1 << (m_composition.NumberSections * m_composition.NumberBitsSection)) -
       1);

  return Base::getPlay(playedNote,
                       {m_composition.NumberSections,
                        m_composition.NumberBitsSection});
}

template <>
bool Custom<InstrumentType::Custom_2, Difficulty::Easy>::getPlay(
    std::uint32_t playedNote) noexcept {

  const std::uint32_t UsedBits =
      (1 << m_composition.NumberEffectiveBitsEasy) - 1;

  playedNote &= UsedBits;

  return Base::getPlay(playedNote);
}

template <>
bool Custom<InstrumentType::Custom_2, Difficulty::Hard>::getPlay(
    std::uint32_t playedNote) noexcept {

  const std::uint32_t UsedBits =
      (1 << m_composition.NumberEffectiveBitsHard) - 1;

  playedNote &= UsedBits;
  return Base::getPlay(playedNote);
}

} // namespace bh
