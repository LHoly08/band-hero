#pragma once

#include "medals/AchievementsManager.hpp"

#include "states/State.hpp"

#include "ui/AnimatedBackground.hpp"
#include "ui/Button.hpp"

namespace bh {

class AchievementsState : public State {
public:
  inline AchievementsState(StateStack &stack) noexcept : State(stack) {}
  ~AchievementsState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<ResourceManager::TextureHandle> m_textures =
      ResourceManager::acquireTextures<Textures::UI, Textures::Achievements>();
  AnimatedBackground m_background;

  Button m_menuButton{{100, 885}, {0, 0, 360, 100}, "Main Menu"};
  std::vector<Achievements> m_medals = AchievementsManager::getEarnedMedals();
};
} // namespace bh
