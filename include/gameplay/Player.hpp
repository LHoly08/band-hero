#pragma once

#include <cstdint>

#include <memory>

#include "gameplay/instruments/Instrument.hpp"

namespace bh {

template <InstrumentType Type, Difficulty Dif> class Player final {
public:
  Player();
  ~Player() = default;

  inline void play(std::uint32_t notePlayed) noexcept {
    m_score += m_instrument->getPlay(notePlayed);
  }

private:
  std::uint32_t m_score{};
  std::uint32_t m_passedNotes{};
  std::unique_ptr<Instrument<Type, Dif>> m_instrument;
};

template <InstrumentType Type, Difficulty Dif> Player<Type, Dif>::Player() {}

} // namespace bh
