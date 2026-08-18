#include "gameplay/instruments/Custom.hpp"

#include "gameplay/instruments/Instrument.hpp"

namespace bh {

template <>
inline bool Custom<InstrumentType::Custom_1, Difficulty::Easy>::getPlay(
    std::uint32_t playedNote) noexcept {

  playedNote &=
      ((1 << (m_composition.NumberSections * m_composition.NumberBitsSection)) -
       1);

  for (std::uint8_t i{}; i < m_composition.NumberSections; ++i) {

    const std::uint8_t shiftAmount{m_composition.NumberBitsSection};

    // Creation of x bits all set to 1
    const std::uint8_t fullBits = (1 << shiftAmount) - 1;

    // Get if section was played
    bool played = (playedNote >> (i * shiftAmount)) & fullBits;

    // Set every bit of the section to 1 if played
    playedNote |= played * (fullBits << (i * shiftAmount));
  }

  return Base::getPlay(playedNote);
}

template <>
inline bool Custom<InstrumentType::Custom_1, Difficulty::Hard>::getPlay(
    std::uint32_t playedNote) noexcept {

  playedNote &=
      ((1 << (m_composition.NumberSections * m_composition.NumberBitsSection)) -
       1);

  return Base::getPlay(playedNote);
}

template <>
inline bool Custom<InstrumentType::Custom_2, Difficulty::Easy>::getPlay(
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
