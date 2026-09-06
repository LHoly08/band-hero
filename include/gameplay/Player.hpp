#pragma once

#include <cassert>
#include <cstdint>

#include <memory>
#include <utility>

#include "raylib.h"

#include "gameplay/instruments/Bass.hpp"
#include "gameplay/instruments/Custom.hpp"
#include "gameplay/instruments/Drums.hpp"
#include "gameplay/instruments/Guitar.hpp"
#include "gameplay/instruments/Instrument.hpp"

namespace bh {

namespace detail {

template <InstrumentType Type, Difficulty Dif, typename... Args>
std::unique_ptr<Instrument<Type, Dif>>
makePlayerInstrument(std::uint32_t &noteCount, Args &&...args) {

  if constexpr (Type == InstrumentType::Bass) {

    return std::make_unique<Bass<Dif>>(noteCount, std::forward<Args>(args)...);

  } else if constexpr (Type == InstrumentType::Drums) {

    return std::make_unique<Drums<Dif>>(noteCount, std::forward<Args>(args)...);

  } else if constexpr (Type == InstrumentType::Guitar) {

    return std::make_unique<Guitar<Dif>>(noteCount,
                                         std::forward<Args>(args)...);

  } else if constexpr (Type == InstrumentType::Custom_1) {

    return std::make_unique<Custom<InstrumentType::Custom_1, Dif>>(
        noteCount, std::forward<Args>(args)...);

  } else {

    return std::make_unique<Custom<InstrumentType::Custom_2, Dif>>(
        noteCount, std::forward<Args>(args)...);
  }
}

} // namespace detail

class PlayerBase {
public:
  virtual ~PlayerBase() = default;

  virtual void draw() const noexcept = 0;
  virtual void update(float dt) noexcept = 0;

  virtual void play(std::uint32_t notePlayed) noexcept = 0;
  virtual void updateInstrumentSpeed(const float &speed) noexcept = 0;

  inline static void setPlayerCount(std::uint8_t count) noexcept {
    assert(count != 0 && count <= 4);
    PlayerCount = count;
  }

protected:
  inline static std::uint8_t PlayerCount = 1;
};

template <InstrumentType Type, Difficulty Dif>
class Player final : public PlayerBase {
public:
  template <typename... Args> Player(std::uint32_t id, Args &&...args);
  ~Player() override = default;
  Player(const Player &) = delete;
  Player &operator=(const Player &) = delete;
  Player(Player &&) = default;
  Player &operator=(Player &&) = default;

  inline void updateInstrumentSpeed(const float &speed) noexcept override {
    m_speed = speed;
    m_instrument->updateSpeed(&m_speed);
  }

  inline void play(std::uint32_t notePlayed) noexcept override {
    m_score += m_instrument->getPlay(notePlayed);
  }

  inline void draw() const noexcept override {
    m_instrument->draw((GetScreenWidth() / PlayerCount) * id);
  }

  inline void update(float dt) noexcept override { m_instrument->update(dt); }

private:
  const std::uint32_t id;

  std::uint32_t m_score{};
  std::uint32_t m_passedNotes{};
  float m_speed;
  std::unique_ptr<Instrument<Type, Dif>> m_instrument;
};

template <InstrumentType Type, Difficulty Dif>
template <typename... Args>
Player<Type, Dif>::Player(std::uint32_t id, Args &&...args)
    : id(id), m_instrument(detail::makePlayerInstrument<Type, Dif>(
                  m_passedNotes, std::forward<Args>(args)...)) {}

} // namespace bh
