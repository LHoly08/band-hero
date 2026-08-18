#include "core/StateStack.hpp"

#include "states/MainMenuState.hpp"

namespace bh {

StateStack::StateStack() {
  m_stack.reserve(3);

  push<MainMenuState>();
  act();
}

void StateStack::act() noexcept {

  switch (action.type) {

  case ActionType::Push: {
    m_stack.back()->onExit();
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
  action.state = nullptr;
  action.type = ActionType::None;
}

} // namespace bh
