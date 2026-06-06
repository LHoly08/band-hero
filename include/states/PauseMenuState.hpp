#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

#include "states/State.hpp"

namespace bh {

class PauseMenuState : public State {
public:
  explicit PauseMenuState(StateStack &stack) noexcept;
  virtual ~PauseMenuState() = default;

  virtual void draw(sf::RenderTarget &target) const noexcept override;
  virtual void handleEvents(const sf::Event &event) noexcept override;
  virtual void update(const float dt) noexcept override;

  virtual void onEnter() noexcept override;
  virtual void onExit() noexcept override;
};

} // namespace bh
