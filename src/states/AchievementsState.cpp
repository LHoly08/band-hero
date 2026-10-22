
#include "states/AchievementsState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "ui/Button.hpp"
#include "ui/Theme.hpp"

namespace bh {

void bh::AchievementsState::draw() const noexcept {
  m_background.draw();
  m_menuButton.draw<WHITE, true, 40, bh::TextAlign::Center>();

  bh::ResourceManager::drawText<bh::Fonts_t::Buttons>(
      "ACHIEVEMENTS", {100, 110}, 48, bh::theme::Text);
  bh::ResourceManager::drawText<bh::Fonts_t::Buttons>(
      std::to_string(m_medals.size()) + " / " +
          std::to_string(bh::AchievementsManager::MedalCount) + " unlocked",
      {100, 175}, 26, bh::theme::Highlight);
  std::string_view hint = "Hover over a medal to see its unlock requirement.";
  for (std::size_t i = 0; i < bh::AchievementsManager::MedalCount; ++i) {
    const auto medal = static_cast<bh::Achievements>(i);
    const bool unlocked = std::ranges::find(m_medals, medal) != m_medals.end();
    const float x = 60 + (i % 9) * 200.f;
    const float y = 270 + (i / 9) * 255.f;
    bh::AchievementsManager::drawMedal(
        medal, ::Rectangle{x + 25, y, 150, 150},
        unlocked ? WHITE : bh::theme::Disabled);
    const auto name = bh::AchievementsManager::name(medal);
    const float width = bh::ResourceManager::measureText<bh::Fonts_t::Buttons>(name, 24);
    const float size = std::min(24.f, 180.f * 24.f / std::max(1.f, width));
    const float fitted = bh::ResourceManager::measureText<bh::Fonts_t::Buttons>(name, size);
    bh::ResourceManager::drawText<bh::Fonts_t::Buttons>(
        name, {x + (200 - fitted) / 2, y + 160}, size,
        unlocked ? bh::theme::Text : bh::theme::MutedText);
    bh::ResourceManager::drawText<bh::Fonts_t::Buttons>(
        unlocked ? "Unlocked" : "Locked", {x + 60, y + 195}, 18,
        unlocked ? bh::theme::Highlight : bh::theme::MutedText);
    const auto position = bh::scaledSize(::Vector2{x, y});
    const auto extent = bh::scaledSize(::Vector2{200, 220});
    if (::CheckCollisionPointRec(::GetMousePosition(),
                               {position.x, position.y, extent.x, extent.y})) {
      hint = bh::AchievementsManager::description(medal);
    }
  }
  bh::ResourceManager::drawText<bh::Fonts_t::Buttons>(
      hint, {100, 815}, 25, bh::theme::MutedText);
}

void bh::AchievementsState::update(float dt) noexcept { m_background.update(dt); }

void bh::AchievementsState::events() noexcept {

  if (m_menuButton.updateInput(::GetMousePosition())) [[unlikely]] {
    m_stack.pop();
  }
}

void bh::AchievementsState::onEnter() noexcept {
  m_medals = bh::AchievementsManager::getEarnedMedals();
}

void bh::AchievementsState::onExit() noexcept { m_menuButton.resetInteraction(); }

} // namespace bh
