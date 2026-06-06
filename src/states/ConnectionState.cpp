
#include <SFML/Graphics/Color.hpp>

#include "states/ConnectionState.hpp"

#include "core/StateStack.hpp"

#include "settings/Device.hpp"

#include "states/MainMenuState.hpp"

namespace bh {

ConnectionState::ConnectionState(StateStack &stack) noexcept : State(stack) {}

void ConnectionState::draw(sf::RenderTarget &target) const noexcept {
  target.clear(sf::Color::Blue);
}

void ConnectionState::handleEvents(const sf::Event &event) noexcept {

  if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {

    switch (keyPressed->scancode) {

    case sf::Keyboard::Scancode::Escape:
      m_stack.pop();
      m_stack.replace<MainMenuState>();
      break;

    default:
      break;
    }
  }
}

void ConnectionState::update(const float dt) noexcept {}

void ConnectionState::onEnter() noexcept {
  Device::ScanPorts();
  auto name = Device::PortName();
  if (name) {
    m_stack.pop();
  }
}

void ConnectionState::onExit() noexcept {}

} // namespace bh
