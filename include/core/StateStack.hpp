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

class StateStack {
public:
  StateStack();
  ~StateStack() = default;

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

  void draw() const noexcept {
    for (auto &state : m_stack) {
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
  inline void clear() noexcept { m_stack.clear(); }

private:
  enum class ActionType : std::uint8_t {
    Push = 0,
    Pop,
    Replace,
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
