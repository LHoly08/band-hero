#include "states/GamemodeState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"
#include "states/PlayerSelectState.hpp"

namespace bh {

void GamemodeState::draw() const noexcept {
  m_localButton.draw();
  m_lanButton.draw();
  m_onlineButton.draw();
}

void GamemodeState::update(float dt) noexcept {}

void GamemodeState::events() noexcept {
  if (IsMouseButtonPressed(0)) {
    const Vector2 MousePos{GetMousePosition()};

    if (m_localButton.pressed(MousePos)) {
      m_stack.replace<PlayerSelectState>();
    } else if (m_lanButton.pressed(MousePos)) {
    } else if (m_onlineButton.pressed(MousePos)) {
    }
  }
}

void GamemodeState::onEnter() noexcept {}

void GamemodeState::onExit() noexcept {}

} // namespace bh
