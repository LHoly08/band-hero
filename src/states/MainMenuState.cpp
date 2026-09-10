#include "states/MainMenuState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "states/GamemodeState.hpp"
#include "states/SettingsState.hpp"

#include "ui/Button.hpp"

namespace bh {

void MainMenuState::draw() const noexcept {
  m_playButton.draw<WHITE, true, 64, TextAlign::Right>();
  m_quitButton.draw<WHITE, true>();
  m_settingsButton.draw();
}

void MainMenuState::update(float dt) noexcept {}

void MainMenuState::events() noexcept {
  if (IsMouseButtonPressed(0)) {
    const Vector2 MousePos{GetMousePosition()};

    if (m_playButton.pressed(MousePos)) {
      m_stack.replace<GamemodeState>();
    } else if (m_quitButton.pressed(MousePos)) {
      m_stack.quit = true;
      return;

    } else if (m_settingsButton.pressed(MousePos)) {
      m_stack.replace<SettingsState>();
    }
  }
}

void MainMenuState::onEnter() noexcept {}

void MainMenuState::onExit() noexcept {}

} // namespace bh
