#pragma once

#include "core/ResourceManager.hpp"

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class MainMenuState final : public bh::State {
public:
  inline MainMenuState(bh::StateStack &stack) noexcept
      : bh::State(stack) {}
  ~MainMenuState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<bh::ResourceManager::TextureHandle> m_textures =
      bh::ResourceManager::acquireTextures<bh::Textures::UI, bh::Textures::MainMenu>();
  bh::Button m_playButton{
    {.x = 240, .y = 440},
    {0, 0, 360, 100},
    "Play"
  };
  bh::Button m_quitButton{
    {.x = 240, .y = 560},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "Quit"
  };
  bh::Button m_settingsButton{
    {.x = 250, .y = 680},
    {.x = 500, .y = 0, .width = 96, .height = 96}
  };
  bh::Button m_guidesButton{
    {.x = 372, .y = 680},
    {.x = 500, .y = 100, .width = 96, .height = 96}
  };
  bh::Button m_achievementsButton{
    {.x = 494, .y = 680}, 
    {.x = 500, .y = 200, .width = 96, .height = 96}
  };
};

} // namespace bh
