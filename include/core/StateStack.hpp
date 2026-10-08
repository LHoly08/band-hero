#pragma once

#include <concepts>
#include <cstdint>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "states/State.hpp"

namespace bh {

template <typename T>
concept DerivedState = std::derived_from<T, State>;

// Transitions are queued until act(), so a state cannot destroy itself while
// its events/update callback is still running. Only the top state is active,
// but all states draw from bottom to top. States own texture references from
// construction to destruction, including while queued or suspended.
// Only the active state has entered: covering it calls onExit(), and revealing
// it again calls onEnter(). Covered states retain textures and remain drawable.
class StateStack {
public:
  StateStack();
  ~StateStack() { clear(); }

  template <DerivedState S, typename... Args> inline void push(Args &&...args) {
    actions.push(
        {.state = std::make_unique<S>(*this, std::forward<Args>(args)...),
         .type = ActionType::Push});
  }

  inline void pop() {
    actions.push({.state = nullptr, .type = ActionType::Pop});
  }

  template <DerivedState S, typename... Args>
  inline void replace(Args &&...args) {
    actions.push(
        {.state = std::make_unique<S>(*this, std::forward<Args>(args)...),
         .type = ActionType::Replace});
  }

  // Replace the entire stack without resuming suspended states.
  template <DerivedState S, typename... Args> inline void reset(Args &&...args) {
    actions.push(
        {.state = std::make_unique<S>(*this, std::forward<Args>(args)...),
         .type = ActionType::Reset});
  }

  void draw() const noexcept {
    for (const auto &state : m_stack) {
      state->draw();
    }
  }
  void update(float dt) noexcept {
    if (!m_stack.empty()) {
      m_stack.back()->update(dt);
    }
  }
  void events() noexcept {
    if (!m_stack.empty()) {
      m_stack.back()->events();
    }
  }
  void act() noexcept;
  void clear() noexcept;

private:
  enum class ActionType : std::uint8_t {
    Push = 0,
    Pop,
    Replace,
    Reset,
    None,
  };

  std::vector<std::unique_ptr<State>> m_stack;

  struct Action {
    std::unique_ptr<State> state{nullptr};
    ActionType type{ActionType::None};
  };
  std::queue<Action> actions;

public:
  bool quit{false};
};

} // namespace bh
