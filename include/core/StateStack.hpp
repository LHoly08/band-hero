#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

#include <concepts>
#include <memory>
#include <queue>
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
    PUSH = 0,
    POP,
    REPLACE,
  };

  struct Action {
    Actions type;
    std::unique_ptr<State> state{nullptr};

    Action(Actions type, std::unique_ptr<State> state)
        : type(type), state(std::move(state)) {}
  };

  std::vector<std::unique_ptr<State>> m_stack;
  std::queue<Action> m_actions;
};

template <StateDerived S, typename... Args>
void StateStack::push(Args &&...args) {
  m_actions.push(
      {Actions::PUSH, std::make_unique<S>(*this, std::forward<Args>(args)...)});
}

template <StateDerived S, typename... Args>
void StateStack::replace(Args &&...args) {

  m_actions.push({Actions::REPLACE,
                  std::make_unique<S>(*this, std::forward<Args>(args)...)});
}

} // namespace bh
