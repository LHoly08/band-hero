#pragma once

#include "states/State.hpp"

namespace bh {

class MainMenuState final : public State {
public:
  inline MainMenuState(StateStack &stack) noexcept : State(stack) {}
  ~MainMenuState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
};
} // namespace bh
