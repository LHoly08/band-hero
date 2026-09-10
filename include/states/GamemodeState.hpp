#pragma once

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class GamemodeState final : public State {
public:
  inline GamemodeState(StateStack &stack) noexcept
      : State(stack),
        m_localButton({.x = 100, .y = 100},
                      {.x = 0, .y = 0, .width = 500, .height = 200}),
        m_lanButton({.x = 500, .y = 500},
                    {.x = 0, .y = 0, .width = 500, .height = 200}),
        m_onlineButton({.x = 1000, .y = 300},
                       {.x = 0, .y = 0, .width = 500, .height = 200}) {}
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
};

} // namespace bh
