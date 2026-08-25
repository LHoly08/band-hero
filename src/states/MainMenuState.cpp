#include "states/MainMenuState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "states/GameState.hpp"

namespace bh {

void MainMenuState::draw() const noexcept { m_playButton.draw(); }

void MainMenuState::update(float dt) noexcept {}

void MainMenuState::events() noexcept {
  if (IsMouseButtonPressed(0)) {
    if (m_playButton.pressed(GetMousePosition())) {
      m_stack.replace<GameState<4>>();
    }
  }
}

void MainMenuState::onEnter() noexcept {}

void MainMenuState::onExit() noexcept {}

} // namespace bh
