#pragma once

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class MainMenuState final : public State {
public:
  inline MainMenuState(StateStack &stack) noexcept
      : State(stack), m_playButton("assets/textures/MainMenu/UI/Button.png",
                                   {.x = 100, .y = 100}) {}
  ~MainMenuState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  Button m_playButton;
};

} // namespace bh
