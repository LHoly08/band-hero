#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

#include <concepts>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "states/State.hpp"

namespace bh {

template <typename S>
concept StateDerived = std::derived_from<S, State>;

class StateStack {
public:
  StateStack();
  ~StateStack() = default;

  StateStack(const StateStack &) = delete;
  void operator=(const StateStack &) = delete;
  StateStack(StateStack &&) = delete;
  void operator=(StateStack &&) = delete;

  template <StateDerived S, typename... Args> void push(Args &&...args);
  void pop() noexcept;
  template <StateDerived S, typename... Args> void replace(Args &&...args);

  void act() noexcept;

  void draw(sf::RenderTarget &target) const noexcept;
  void handleEvents(const sf::Event &event) noexcept;
  void update(float dt) noexcept;

  inline bool empty() const noexcept { return m_stack.empty(); }

private:
  enum class Actions {
    NONE = 0,
    PUSH,
    POP,
    REPLACE,
  };

  struct Action {
    Actions type{Actions::NONE};
    std::optional<std::unique_ptr<State>> state{std::nullopt};

    inline void reset() noexcept {
      type = Actions::NONE;
      state = std::nullopt;
    }
  };

  std::vector<std::unique_ptr<State>> m_stack;
  Action m_action;
};

template <StateDerived S, typename... Args>
void StateStack::push(Args &&...args) {

  m_action = {Actions::PUSH,
              std::make_unique<S>(*this, std::forward<Args>(args)...)};
}

template <StateDerived S, typename... Args>
void StateStack::replace(Args &&...args) {

  m_action = {Actions::REPLACE,
              std::make_unique<S>(*this, std::forward<Args>(args)...)};
}

} // namespace bh
