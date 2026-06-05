#include "states/PauseMenuState.hpp"

#include "core/StateStack.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace bh {

PauseMenuState::PauseMenuState(StateStack &stack) noexcept : State(stack) {}

void PauseMenuState::draw(sf::RenderTarget &target) const noexcept {}

void PauseMenuState::handleEvents(const sf::Event &event) noexcept {

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

void PauseMenuState::update(const float dt) noexcept {}

void PauseMenuState::onEnter() noexcept {}

void PauseMenuState::onExit() noexcept {}

} // namespace bh
