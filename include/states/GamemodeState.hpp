#pragma once

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class GamemodeState final : public State {
public:
  inline GamemodeState(StateStack &stack) noexcept
      : State(stack), m_localButton("assets/textures/MainMenu/UI/Button.png",
                                    {.x = 100, .y = 100}),
        m_lanButton("assets/textures/MainMenu/UI/Button.png",
                    {.x = 500, .y = 500}),
        m_onlineButton("assets/textures/MainMenu/UI/Button.png",
                       {.x = 1000, .y = 300}) {}
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
