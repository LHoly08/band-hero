
#pragma once

#include "serial/serialib.h"

#include "states/State.hpp"

namespace bh {

class GameState final : public State {
public:
  inline GameState(StateStack &stack) noexcept : State(stack) {}
  ~GameState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  serialib m_serial;
};
} // namespace bh
