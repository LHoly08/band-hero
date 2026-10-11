
#include "states/AchievementsState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "ui/Button.hpp"
#include "ui/Theme.hpp"

namespace bh {

void AchievementsState::draw() const noexcept {
  m_background.draw();
  m_menuButton.draw<WHITE, true, 40, TextAlign::Center>();

  ResourceManager::drawText<Fonts_t::Buttons>(
      "ACHIEVEMENTS", {100, 110}, 48, theme::Text);
  ResourceManager::drawText<Fonts_t::Buttons>(
      std::to_string(m_medals.size()) + " / " +
          std::to_string(AchievementsManager::MedalCount) + " unlocked",
      {100, 175}, 26, theme::Highlight);
  std::string_view hint = "Hover over a medal to see its unlock requirement.";
  for (std::size_t i = 0; i < AchievementsManager::MedalCount; ++i) {
    const auto medal = static_cast<Achievements>(i);
    const bool unlocked = std::ranges::find(m_medals, medal) != m_medals.end();
    const float x = 60 + (i % 9) * 200.f;
    const float y = 270 + (i / 9) * 255.f;
    AchievementsManager::drawMedal(
        medal, Rectangle{x + 25, y, 150, 150},
        unlocked ? WHITE : theme::Disabled);
    const auto name = AchievementsManager::name(medal);
    const float width = ResourceManager::measureText<Fonts_t::Buttons>(name, 24);
    const float size = std::min(24.f, 180.f * 24.f / std::max(1.f, width));
    const float fitted = ResourceManager::measureText<Fonts_t::Buttons>(name, size);
    ResourceManager::drawText<Fonts_t::Buttons>(
        name, {x + (200 - fitted) / 2, y + 160}, size,
        unlocked ? theme::Text : theme::MutedText);
    ResourceManager::drawText<Fonts_t::Buttons>(
        unlocked ? "Unlocked" : "Locked", {x + 60, y + 195}, 18,
        unlocked ? theme::Highlight : theme::MutedText);
    const auto position = scaledSize(Vector2{x, y});
    const auto extent = scaledSize(Vector2{200, 220});
    if (CheckCollisionPointRec(GetMousePosition(),
                               {position.x, position.y, extent.x, extent.y})) {
      hint = AchievementsManager::description(medal);
    }
  }
  ResourceManager::drawText<Fonts_t::Buttons>(
      hint, {100, 815}, 25, theme::MutedText);
}

void AchievementsState::update(float dt) noexcept { m_background.update(dt); }

void AchievementsState::events() noexcept {

  if (m_menuButton.updateInput(GetMousePosition())) [[unlikely]] {
    m_stack.pop();
  }
}

void AchievementsState::onEnter() noexcept {
  m_medals = AchievementsManager::getEarnedMedals();
}

void AchievementsState::onExit() noexcept { m_menuButton.resetInteraction(); }

} // namespace bh
