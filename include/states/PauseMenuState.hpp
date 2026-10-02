#pragma once

#include "core/StateStack.hpp"

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class PauseMenuState final : public State {
public:
  inline PauseMenuState(StateStack &stack) noexcept
      : State(stack),
        m_continueButton({.x = 780, .y = 370},
                         {.x = 0, .y = 0, .width = 360, .height = 100},
                         "Continue"),
        m_mainMenuButton({.x = 780, .y = 490},
                         {.x = 0, .y = 0, .width = 360, .height = 100},
                         "Main Menu"),
        m_quitButton({.x = 780, .y = 610},
                     {.x = 0, .y = 0, .width = 360, .height = 100}, "Quit") {}
  ~PauseMenuState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  Button m_continueButton;
  Button m_mainMenuButton;
  Button m_quitButton;

  float m_delay{};
  bool m_clicked{false};
};

} // namespace bh
