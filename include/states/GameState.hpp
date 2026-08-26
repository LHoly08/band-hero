#pragma once

#include <cstdint>

#include <array>
#include <memory>
#include <utility>

#include "gameplay/Player.hpp"

#include "serial/serialib.h"

#include "states/State.hpp"

namespace bh {

template <std::uint8_t T>
concept MaxPlayerAmount = (T != 0) && (T <= 4);

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
class GameState final : public State {
public:
  inline GameState(
      StateStack &stack,
      std::array<std::unique_ptr<PlayerBase>, PlayerCount> &&players) noexcept
      : State(stack), m_players(std::move(players)) {}
  ~GameState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  serialib m_serial;
  std::array<std::unique_ptr<PlayerBase>, PlayerCount> m_players;
};

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::draw() const noexcept {}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::update(float dt) noexcept {

  if (std::uint32_t buffer{}; m_serial.readBytes(&buffer, sizeof(buffer), 1)) {

    if (std::uint8_t index = buffer & 0b11; index < PlayerCount) {
      buffer <<= 2;
      m_players[index]->play(buffer);
    }
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::events() noexcept {}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::onEnter() noexcept {}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::onExit() noexcept {}

} // namespace bh
