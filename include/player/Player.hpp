#pragma once
#include "instruments/Instrument.hpp"
#include <concepts>
#include <cstdint>
#include <memory>

namespace bh {

template <Difficulty Dif> class Player {
public:
  explicit Player(std::unique_ptr<Instrument<Dif>> instrument);

  ~Player() = default;
  Player(const Player &) = delete;
  void operator=(const Player &) = delete;
  Player(Player &&) = default;
  Player &operator=(Player &&) = default;

  void play(std::uint32_t &val);
  void getScore();

  inline void startThread() noexcept { m_instrument->startThread(); }
  inline void pauseThread() noexcept { m_instrument->pauseThread(); }

private:
  std::unique_ptr<Instrument<Dif>> m_instrument{};
  unsigned int m_totalScore{};
  unsigned int m_score{};
};

template <Difficulty Dif>
Player<Dif>::Player(std::unique_ptr<Instrument<Dif>> instrument)
    : m_instrument(std::move(instrument)) {}

} // namespace bh