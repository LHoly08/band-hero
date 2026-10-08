#pragma once

#include "core/StateStack.hpp"

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class PauseMenuState final : public State {
public:
  inline PauseMenuState(StateStack &stack) noexcept
      : State(stack) {}
  ~PauseMenuState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<ResourceManager::TextureHandle> m_textures =
      ResourceManager::acquireTextures<Textures::UI>();
  Button m_continueButton{
    {.x = 590, .y = 430},
    {.x = 0, .y = 300, .width = 740, .height = 100},
    "Continue"
  };
  Button m_mainMenuButton{
    {.x = 590, .y = 550},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "Main Menu"
  };
  Button m_quitButton{
    {.x = 970, .y = 550},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "Quit"
  };

  float m_delay{};
  bool m_clicked{false};
};

} // namespace bh
