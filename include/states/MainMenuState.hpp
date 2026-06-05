#pragma once
#include <SFML/Graphics/RenderTarget.hpp>

#include "states/State.hpp"

namespace bh
{

  class MainMenuState : public State
  {
  public:
    explicit MainMenuState(StateStack &stack) noexcept;
    virtual ~MainMenuState() = default;

    virtual void draw(sf::RenderTarget &target) const noexcept override;
    virtual void handleEvents(const sf::Event &event) noexcept override;
    virtual void update(const float dt) noexcept override;

    virtual void onEnter() noexcept override;
    virtual void onExit() noexcept override;
  };

} // namespace bh
