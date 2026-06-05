#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

namespace bh
{

  class StateStack;

  class State
  {
  public:
    explicit State(StateStack &stack) noexcept;
    virtual ~State() = default;

    virtual void draw(sf::RenderTarget &target) const noexcept = 0;
    virtual void handleEvents(const sf::Event &event) noexcept = 0;
    virtual void update(const float dt) noexcept = 0;
    virtual void onExit() noexcept = 0;
    virtual void onEnter() noexcept = 0;

  protected:
    StateStack &m_stack;
  };
} // namespace bh
