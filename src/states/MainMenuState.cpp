#include "states/MainMenuState.hpp"

#include "core/StateStack.hpp"

namespace bh {

void MainMenuState::draw() const noexcept { m_playButton.draw(); }

void MainMenuState::update(float dt) noexcept {}

void MainMenuState::events() noexcept {}

void MainMenuState::onEnter() noexcept {}

void MainMenuState::onExit() noexcept {}

} // namespace bh
