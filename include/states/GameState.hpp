#pragma once
#include "states/State.hpp"

#include "instruments/Bass.hpp"
#include "instruments/Drums.hpp"
#include "instruments/Guitar.hpp"
#include "instruments/Instrument.hpp"

#include "serial/serialib.h"
#include "settings/Device.hpp"

#include "core/StateStack.hpp"

#include <SFML/Window/Event.hpp>

#include <memory>
#include <optional>

namespace bh {

template <Difficulty Dif> class GameState : public State {
public:
  explicit GameState(StateStack &stack, std::optional<Bass<Dif>> bass,
                     std::optional<Guitar<Dif>> guitar,
                     std::optional<Drums<Dif>> drums,
                     std::unique_ptr<Instrument> instrumet) noexcept;
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
                          std::unique_ptr<Instrument> instrumet) noexcept
    : State(stack), m_custom(std::move(instrumet)), m_bass(std::move(bass)),
      m_guitar(std::move(guitar)), m_drums(std::move(drums)) {

  m_serial.openDevice(Device::PortName().c_str(), Device::BaudRate());
}

template <Difficulty Dif>
void GameState<Dif>::handleEvents(const sf::Event &event) noexcept {

  if (const auto *key = event.getIf<sf::Event::KeyPressed>()) {

    switch (key->scancode) {
    case sf::Keyboard::Scancode::Escape:
      m_stack.pop();
      break;

    default:
      break;
    }
  }
}

template <Difficulty Dif> void GameState<Dif>::onEnter() noexcept {}

template <Difficulty Dif> void GameState<Dif>::onExit() noexcept {}

} // namespace bh