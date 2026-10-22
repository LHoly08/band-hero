#pragma once

#include "states/State.hpp"
#include "ui/AnimatedBackground.hpp"

#include "ui/Button.hpp"
namespace bh {

class GamemodeState final : public bh::State {
public:
  inline GamemodeState(bh::StateStack &stack) noexcept
      : bh::State(stack) {}
  ~GamemodeState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<bh::ResourceManager::TextureHandle> m_textures =
      bh::ResourceManager::acquireTextures<bh::Textures::UI>();
  bh::AnimatedBackground m_background;

  bh::Button m_localButton{
    {.x = 590, .y = 370},
    {.x = 0, .y = 300, .width = 740, .height = 100},
    "Local"
  };
  bh::Button m_lanButton{
    {.x = 590, .y = 490},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "LAN"
  };
  bh::Button m_onlineButton{
    {.x = 970, .y = 490},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "Online"
  };
  bh::Button m_backButton{
    {.x = 780, .y = 610},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "Main Menu"
  };
};

} // namespace bh
