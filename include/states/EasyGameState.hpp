#pragma once
#include "serial/serialib.h"

#include "states/State.hpp"

namespace bh {
class EasyGameState : public State {
public:
  explicit EasyGameState(StateStack &stack) noexcept;
  virtual ~EasyGameState() = default;

  virtual void draw(sf::RenderTarget &target) const noexcept override;
  virtual void handleEvents(const sf::Event &event) noexcept override;
  virtual void update(float dt) noexcept override;

  virtual void onEnter() noexcept override;
  virtual void onExit() noexcept override;

private:
  serialib m_serial;
};
} // namespace bh
