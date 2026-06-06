#include <SFML/Window/Keyboard.hpp>

#include <optional>

#include "states/MainMenuState.hpp"

#include "core/StateStack.hpp"

#include "instruments/Instrument.hpp"

#include "states/GameState.hpp"

namespace bh {

MainMenuState::MainMenuState(StateStack &stack) noexcept : State(stack) {}

void MainMenuState::draw(sf::RenderTarget &target) const noexcept {}

void MainMenuState::handleEvents(const sf::Event &event) noexcept {

  if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {

    switch (keyPressed->scancode) {
    case sf::Keyboard::Scancode::Escape:
      m_stack.push<GameState<Difficulty::EASY>>(std::nullopt, std::nullopt,
                                                std::nullopt, nullptr);
      break;
    case sf::Keyboard::Scancode::E:
      m_stack.push<GameState<Difficulty::HARD>>(std::nullopt, std::nullopt,
                                                std::nullopt, nullptr);
      break;

    default:
      break;
    }
  }
}

void MainMenuState::update(const float dt) noexcept {}

void MainMenuState::onEnter() noexcept {}

void MainMenuState::onExit() noexcept {}

} // namespace bh
