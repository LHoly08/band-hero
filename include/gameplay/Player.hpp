#pragma once

#include <cstdint>

#include <memory>

#include "gameplay/instruments/Bass.hpp"
#include "gameplay/instruments/Custom.hpp"
#include "gameplay/instruments/Drums.hpp"
#include "gameplay/instruments/Guitar.hpp"
#include "gameplay/instruments/Instrument.hpp"

namespace bh {

namespace detail {

template <InstrumentType Type, Difficulty Dif>
std::unique_ptr<Instrument<Type, Dif>>
makePlayerInstrument(std::uint32_t &noteCount) {
  if constexpr (Type == InstrumentType::Bass) {
    return std::make_unique<Bass<Dif>>(noteCount);
  } else if constexpr (Type == InstrumentType::Drums) {
    return std::make_unique<Drums<Dif>>(noteCount);
  } else if constexpr (Type == InstrumentType::Guitar) {
    return std::make_unique<Guitar<Dif>>(noteCount);
  } else if constexpr (Type == InstrumentType::Custom_1) {
    return std::make_unique<Custom<InstrumentType::Custom_1, Dif>>(noteCount);
  } else {
    return std::make_unique<Custom<InstrumentType::Custom_2, Dif>>(noteCount);
  }
}

} // namespace detail

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

template <InstrumentType Type, Difficulty Dif>
Player<Type, Dif>::Player()
    : m_instrument(detail::makePlayerInstrument<Type, Dif>(m_passedNotes)) {}

} // namespace bh
