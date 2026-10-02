#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <memory>
#include <utility>

#include "raylib.h"

#include "config/Settings.hpp"

#include "core/ResourceManager.hpp"

#include "gameplay/Player.hpp"

#include "serial/serialib.h"

#include "states/PauseMenuState.hpp"
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
void GameState<PlayerCount>::draw() const noexcept {
  for (const auto &player : m_players) {
    player->draw();
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::update(float dt) noexcept {

  for (auto &player : m_players) {
    player->update(dt);
  }

  if (std::uint32_t buffer{};
      m_serial.readBytes(&buffer, sizeof(buffer), 1) == sizeof(buffer)) {

    if constexpr (std::endian::native == std::endian::big) {
      buffer = std::byteswap(buffer);
    }

    if (std::uint8_t index = buffer & 0b11; index < PlayerCount) [[likely]] {
      buffer >>= 2;

      m_players[index]->play(buffer);
    }
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::events() noexcept {

  if (IsKeyPressed(KEY_ESCAPE)) [[unlikely]] {
    m_stack.push<PauseMenuState>();
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::onEnter() noexcept {
  ResourceManager::loadTexture<Textures::Gameplay::Notes>();

  m_serial.openDevice(Settings::getSerialPort().c_str(),
                      Settings::getSerialBaudRate());

  for (auto &player : m_players) {
    player->pauseInstrument(false);
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::onExit() noexcept {
  ResourceManager::unloadTexture<Textures::Gameplay::Notes>();

  m_serial.closeDevice();

  for (auto &player : m_players) {
    player->pauseInstrument(true);
  }
}

} // namespace bh
