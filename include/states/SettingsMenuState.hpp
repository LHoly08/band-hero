#pragma once

#include "states/State.hpp"

namespace bh {

class SettingsMenuState final : public State {
public:
  explicit SettingsMenuState(StateStack &stack) noexcept;
  ~SettingsMenuState() = default;

  void draw(sf::RenderTarget &target) const noexcept override;
  void handleEvents(const sf::Event &event) noexcept override;
  void update(const float dt) noexcept override;

  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
};

} // namespace bh