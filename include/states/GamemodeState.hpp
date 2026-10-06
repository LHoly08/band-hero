#pragma once

#include "states/State.hpp"
#include "ui/AnimatedBackground.hpp"

#include "ui/Button.hpp"
namespace bh {

class GamemodeState final : public State {
public:
  inline GamemodeState(StateStack &stack) noexcept
      : State(stack) {}
  ~GamemodeState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  AnimatedBackground m_background;

  Button m_localButton{
    {.x = 590, .y = 370},
    {.x = 0, .y = 300, .width = 740, .height = 100},
    "Local"
  };
  Button m_lanButton{
    {.x = 590, .y = 490},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "LAN"
  };
  Button m_onlineButton{
    {.x = 970, .y = 490},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "Online"
  };
  Button m_backButton{
    {.x = 780, .y = 610},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "Main Menu"
  };
};

} // namespace bh
