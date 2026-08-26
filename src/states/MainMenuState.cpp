#include "states/MainMenuState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "states/GamemodeState.hpp"

namespace bh {

void MainMenuState::draw() const noexcept {
  m_playButton.draw();
  m_quitButton.draw();
  m_settingsButton.draw();
}

void MainMenuState::update(float dt) noexcept {}

void MainMenuState::events() noexcept {
  if (IsMouseButtonPressed(0)) {
    const Vector2 MousePos{GetMousePosition()};

    if (m_playButton.pressed(MousePos)) {
      m_stack.replace<GamemodeState>();
    } else if (m_quitButton.pressed(MousePos)) {
      WindowShouldClose();
    } else if (m_settingsButton.pressed(MousePos)) {
    }
  }
}

void MainMenuState::onEnter() noexcept {}

void MainMenuState::onExit() noexcept {}

} // namespace bh
