#pragma once

#include "core/ResourceManager.hpp"

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class MainMenuState final : public State {
public:
  inline MainMenuState(StateStack &stack) noexcept
      : State(stack),
        m_playButton({.x = 240, .y = 440}, {0, 0, 360, 100}, "Play"),
        m_quitButton({.x = 240, .y = 560},
                     {.x = 0, .y = 0, .width = 360, .height = 100}, "Quit"),
        m_settingsButton({.x = 240, .y = 680},
                         {.x = 500, .y = 0, .width = 96, .height = 96}) {}
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
