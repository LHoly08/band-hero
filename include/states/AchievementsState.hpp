#pragma once

#include "medals/AchievementsManager.hpp"

#include "states/State.hpp"

#include "ui/AnimatedBackground.hpp"
#include "ui/Button.hpp"

namespace bh {

class AchievementsState : public bh::State {
public:
  inline AchievementsState(bh::StateStack &stack) noexcept : bh::State(stack) {}
  ~AchievementsState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<bh::ResourceManager::TextureHandle> m_textures =
      bh::ResourceManager::acquireTextures<bh::Textures::UI, bh::Textures::Achievements>();
  bh::AnimatedBackground m_background;

  bh::Button m_menuButton{{100, 885}, {0, 0, 360, 100}, "Main Menu"};
  std::vector<bh::Achievements> m_medals = bh::AchievementsManager::getEarnedMedals();
};
} // namespace bh
