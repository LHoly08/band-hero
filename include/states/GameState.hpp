#pragma once
#include <SFML/Window/Event.hpp>

#include <memory>
#include <optional>

#include "core/StateStack.hpp"

#include "instruments/Bass.hpp"
#include "instruments/Drums.hpp"
#include "instruments/Guitar.hpp"
#include "instruments/Instrument.hpp"

#include "serial/serialib.h"

#include "settings/Device.hpp"

#include "states/ConnectionState.hpp"
#include "states/PauseMenuState.hpp"
#include "states/State.hpp"

namespace bh {

template <Difficulty Dif> class GameState final : public State {
public:
  explicit GameState(StateStack &stack, std::string songName,
                     std::unique_ptr<Instrument<Dif>> instrumet);
  ~GameState() = default;

  void draw(sf::RenderTarget &target) const noexcept override;
  void handleEvents(const sf::Event &event) noexcept override;
  void update(const float dt) noexcept override;

  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  serialib m_serial;

  std::array<Player<Dif>, 4> m_players;
  const std::uint8_t m_playerCount;
};

template <Difficulty Dif>
GameState<Dif>::GameState(StateStack &stack, std::string songName, std::uint8_t playerCount)
    : State(stack), m_playerCount(playerCount) {}

template <Difficulty Dif>
void GameState<Dif>::handleEvents(const sf::Event &event) noexcept {

  if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {

    switch (keyPressed->scancode) {
    case sf::Keyboard::Scancode::Escape:
      m_stack.push<PauseMenuState>();
      break;

    default:
      break;
    }
  }
}

template <Difficulty Dif> void GameState<Dif>::update(float dt) noexcept {

  if (std::array<unsigned char, 4> buffer{};
      m_serial.readBytes(buffer.data(), buffer.size(), 1)) {

    std::uint32_t val = std::bit_cast<std::uint32_t>(buffer);

    std::uint8_t ind {static_cast<std::uint8_t>(val & 0b11)};
    val >>= 2;
    
    if (ind < m_playerCount) {
      m_players[ind].play(val);
    }
  }
}

template <Difficulty Dif> void GameState<Dif>::onEnter() noexcept {

  auto serialPort = Device::PortName();

  if (serialPort) {
    m_serial.openDevice(Device::PortName()->c_str(), Device::BaudRate());
  } else {
    m_stack.push<ConnectionState>();
  }
}

template <Difficulty Dif> void GameState<Dif>::onExit() noexcept {}

} // namespace bh
