#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

#include "states/State.hpp"

namespace bh {

class PauseMenuState final : public State {
public:
  explicit PauseMenuState(StateStack &stack) noexcept;
  ~PauseMenuState() = default;

  void draw(sf::RenderTarget &target) const noexcept override;
  void handleEvents(const sf::Event &event) noexcept override;
  void update(const float dt) noexcept override;

  void onEnter() noexcept override;
  void onExit() noexcept override;
};

} // namespace bh
