
#include "states/AchievementsState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "ui/Button.hpp"

namespace bh {

void AchievementsState::draw() const noexcept {
  m_background.draw();
  m_menuButton.draw<WHITE, true, 40, TextAlign::Center>();
}

void AchievementsState::update(float dt) noexcept { m_background.update(dt); }

void AchievementsState::events() noexcept {

  if (m_menuButton.updateInput(GetMousePosition())) [[unlikely]] {
    m_stack.pop();
  }
}

void AchievementsState::onEnter() noexcept {}

void AchievementsState::onExit() noexcept { m_menuButton.resetInteraction(); }

} // namespace bh
