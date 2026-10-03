#pragma once

#include "states/State.hpp"

#include "ui/Button.hpp"
namespace bh {

class GamemodeState final : public State {
public:
  inline GamemodeState(StateStack &stack) noexcept
      : State(stack),
        m_localButton({.x = 590, .y = 370},
                      {.x = 0, .y = 300, .width = 740, .height = 100}, "Local"),
        m_lanButton({.x = 590, .y = 490},
                    {.x = 0, .y = 0, .width = 360, .height = 100}, "LAN"),
        m_onlineButton({.x = 970, .y = 490},
                       {.x = 0, .y = 0, .width = 360, .height = 100}, "Online"),
        m_backButton({.x = 780, .y = 610},
                     {.x = 0, .y = 0, .width = 360, .height = 100},
                     "Main Menu") {}
  ~GamemodeState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  Button m_localButton;
  Button m_lanButton;
  Button m_onlineButton;
  Button m_backButton;
};

} // namespace bh
