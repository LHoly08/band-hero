#include "core/StateStack.hpp"

namespace bh {

StateStack::StateStack() { m_stack.reserve(3); }

void StateStack::clear() noexcept {
  if (!m_stack.empty()) {
    m_stack.back()->onExit();
  }
  m_stack.clear();
  // Pending states have not entered and therefore own no GPU resources.
  while (!actions.empty()) {
    actions.pop();
  }
}

void StateStack::act() noexcept {

  while (!actions.empty()) {
    Action action = std::move(actions.front());
    actions.pop();

    switch (action.type) {

    case ActionType::Push: {
      if (!m_stack.empty()) {
        m_stack.back()->onExit();
      }
      m_stack.push_back(std::move(action.state));
      m_stack.back()->onEnter();
      break;
    }
    case ActionType::Pop: {
      if (m_stack.size() > 1) {
        m_stack.back()->onExit();
        m_stack.pop_back();
        m_stack.back()->onEnter();
      }
      break;
    }
    case ActionType::Replace: {
      if (!m_stack.empty()) {
        m_stack.back()->onExit();
        m_stack.pop_back();
      }
      m_stack.push_back(std::move(action.state));
      m_stack.back()->onEnter();
      break;
    }
    case ActionType::None: {
      break;
    }
    }
  }
}

} // namespace bh
