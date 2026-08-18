#pragma once

#include <cstdint>

#include <concepts>
#include <memory>
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
    action.type = ActionType::Push;
    action.state = std::make_unique<S>(*this, std::forward<Args>(args)...);
  }

  inline void pop() {
    action.type = ActionType::Pop;
    action.state = nullptr;
  }

  template <DerivedState S, typename... Args>
  inline void replace(Args &&...args) {
    action.type = ActionType::Replace;
    action.state = std::make_unique<S>(*this, std::forward<Args>(args)...);
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

private:
  enum class ActionType : std::uint8_t {
    Push = 0,
    Pop,
    Replace,
    None,
  };
  struct Action {
    std::unique_ptr<State> state{nullptr};
    ActionType type{ActionType::None};
  } action;

  std::vector<std::unique_ptr<State>> m_stack;
};

} // namespace bh
