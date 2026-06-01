#include "states/MainMenuState.hpp"

#include "core/StateStack.hpp"

namespace bh {

MainMenuState::MainMenuState(StateStack &stack) noexcept : State(stack) {}

void MainMenuState::draw(sf::RenderTarget &target) const noexcept {}
void MainMenuState::handleEvents(const sf::Event &event) noexcept {}
void MainMenuState::update(float dt) noexcept {}

void MainMenuState::onEnter() noexcept {}
void MainMenuState::onExit() noexcept {}

} // namespace bh
