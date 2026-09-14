#include "states/GamemodeState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "states/PlayerSelectState.hpp"

namespace bh {

void GamemodeState::draw() const noexcept {
  m_localButton.draw<WHITE, true>();
  m_lanButton.draw<WHITE, true>();
  m_onlineButton.draw<WHITE, true>();
}

void GamemodeState::update(float dt) noexcept {}

void GamemodeState::events() noexcept {
  if (IsMouseButtonPressed(0)) {
    const Vector2 MousePos{GetMousePosition()};

    if (m_localButton.pressed(MousePos)) [[unlikely]] {
      m_stack.replace<PlayerSelectState>();
    } else if (m_lanButton.pressed(MousePos)) [[unlikely]] {

    } else if (m_onlineButton.pressed(MousePos)) [[unlikely]] {
    }
  }
}

void GamemodeState::onEnter() noexcept {}

void GamemodeState::onExit() noexcept {}

} // namespace bh
