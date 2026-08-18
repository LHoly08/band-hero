#pragma once

namespace bh {

class StateStack;

class State {
public:
  inline State(StateStack &stack) : m_stack(stack) {}
  virtual ~State() = default;

  virtual void draw() const noexcept = 0;
  virtual void update(float dt) noexcept = 0;
  virtual void events() noexcept = 0;
  virtual void onEnter() noexcept = 0;
  virtual void onExit() noexcept = 0;

protected:
  StateStack &m_stack;
};
} // namespace bh
