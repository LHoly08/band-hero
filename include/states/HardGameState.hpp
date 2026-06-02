#pragma once
#include <optional>

#include "instruments/Bass.hpp"
#include "instruments/Instrument.hpp"

#include "serial/serialib.h"

#include "states/State.hpp"

namespace bh {
class HardGameState : public State {
public:
  explicit HardGameState(StateStack &stack) noexcept;
  virtual ~HardGameState() = default;

  virtual void draw(sf::RenderTarget &target) const noexcept override;
  virtual void handleEvents(const sf::Event &event) noexcept override;
  virtual void update(float dt) noexcept override;

  virtual void onEnter() noexcept override;
  virtual void onExit() noexcept override;

private:
  serialib m_serial;

  std::optional<Bass<Difficulty::HARD, 4>> m_bass;
};

} // namespace bh
