#include "states/SettingsMenuState.hpp"
#include "core/StateStack.hpp"
#include "states/MainMenuState.hpp"

#include <fstream>

namespace bh {

SettingsMenuState::SettingsMenuState(StateStack &stack) noexcept
    : State(stack) {}

void SettingsMenuState::draw(sf::RenderTarget &target) const noexcept {}

void SettingsMenuState::handleEvents(const sf::Event &event) noexcept {

  if (const auto *mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
  }
}

void SettingsMenuState::update(const float dt) noexcept {}

void SettingsMenuState::onEnter() noexcept {}

void SettingsMenuState::onExit() noexcept {}

} // namespace bh