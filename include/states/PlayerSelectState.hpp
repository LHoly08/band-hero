#pragma once

#include <filesystem>
#include <inplace_vector>
#include <memory>
#include <variant>

#include "gameplay/Player.hpp"

#include "gameplay/instruments/Custom.hpp"
#include "gameplay/instruments/Instrument.hpp"

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class PlayerSelectState final : public State {
public:
  inline PlayerSelectState(StateStack &stack) noexcept
      : State(stack), m_backNextButton({.x = 100, .y = 100}, {0, 0, 500, 200}),
        m_increaseCountButton({.x = 500, .y = 500}, {0, 0, 500, 200}),
        m_decreaseCountButton({.x = 1000, .y = 300}, {0, 0, 500, 200}),
        m_startButton({.x = 800, .y = 600}, {0, 0, 500, 200}) {}

  ~PlayerSelectState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  Button m_backNextButton;
  Button m_increaseCountButton;
  Button m_decreaseCountButton;
  Button m_startButton;

  std::vector<CustomInstrumentComposition> m_customInstruments;

  std::inplace_vector<std::unique_ptr<PlayerBase>, 4> m_players;
  std::inplace_vector<std::uint8_t, 4> m_playerChoices;

  bool m_choosingCount{true};
  char m_playerCount{'1'};
};

} // namespace bh
