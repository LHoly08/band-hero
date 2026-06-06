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

template <Difficulty Dif> class GameState : public State {
public:
  explicit GameState(StateStack &stack, std::optional<Bass<Dif>> bass,
                     std::optional<Guitar<Dif>> guitar,
                     std::optional<Drums<Dif>> drums,
                     std::unique_ptr<Instrument> instrumet);
  virtual ~GameState() = default;

  virtual void draw(sf::RenderTarget &target) const noexcept override;
  virtual void handleEvents(const sf::Event &event) noexcept override;
  virtual void update(const float dt) noexcept override;

  virtual void onEnter() noexcept override;
  virtual void onExit() noexcept override;

private:
  std::unique_ptr<Instrument> m_custom;

  serialib m_serial;

  std::optional<Bass<Dif>> m_bass;
  std::optional<Guitar<Dif>> m_guitar;
  std::optional<Drums<Dif>> m_drums;
};

template <Difficulty Dif>
GameState<Dif>::GameState(StateStack &stack, std::optional<Bass<Dif>> bass,
                          std::optional<Guitar<Dif>> guitar,
                          std::optional<Drums<Dif>> drums,
                          std::unique_ptr<Instrument> instrumet)
    : State(stack), m_custom(std::move(instrumet)), m_bass(std::move(bass)),
      m_guitar(std::move(guitar)), m_drums(std::move(drums)) {}

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
