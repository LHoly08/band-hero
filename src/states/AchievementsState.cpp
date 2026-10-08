
#include "states/AchievementsState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "ui/Button.hpp"

namespace bh {

void AchievementsState::draw() const noexcept {
  m_background.draw();
  m_menuButton.draw<WHITE, true, 40, TextAlign::Center>();

  static constexpr auto enums =
      std::define_static_array(std::meta::enumerators_of(^^Achievements));

  template for (constexpr auto e : enums) {
    constexpr Achievements a = std::meta::extract<Achievements>(e);
    const std::uint8_t i = static_cast<std::uint8_t>(a);

    AchievementsManager::drawMedal(a, {200 * (i % 9), 200 * (i / 9)});
  }
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
