#pragma once
#include <SFML/Graphics/RenderTarget.hpp>

#include "states/State.hpp"

namespace bh {

class MainMenuState final : public State {
public:
  explicit MainMenuState(StateStack &stack) noexcept;
  ~MainMenuState() = default;

  void draw(sf::RenderTarget &target) const noexcept override;
  void handleEvents(const sf::Event &event) noexcept override;
  void update(const float dt) noexcept override;

  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
};

} // namespace bh
