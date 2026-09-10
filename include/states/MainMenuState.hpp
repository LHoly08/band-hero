#pragma once

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class MainMenuState final : public State {
public:
  inline MainMenuState(StateStack &stack) noexcept
      : State(stack),
        m_playButton({.x = 100, .y = 100}, {0, 0, 500, 200}, "Play"),
        m_quitButton({.x = 500, .y = 500},
                     {.x = 0, .y = 0, .width = 500, .height = 200}, "Quit"),
        m_settingsButton({.x = 1000, .y = 300},
                         {.x = 500, .y = 0, .width = 131, .height = 131}) {}
  ~MainMenuState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  Button m_playButton;
  Button m_quitButton;
  Button m_settingsButton;
};

} // namespace bh
