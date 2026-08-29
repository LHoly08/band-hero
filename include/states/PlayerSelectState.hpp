#pragma once

#include "gameplay/instruments/Custom.hpp"
#include "gameplay/instruments/Instrument.hpp"
#include "states/State.hpp"

#include "gameplay/Player.hpp"
#include "ui/Button.hpp"
#include <filesystem>
#include <inplace_vector>
#include <memory>
#include <variant>

namespace bh {

class PlayerSelectState final : public State {
public:
  inline PlayerSelectState(StateStack &stack) noexcept
      : State(stack), m_backNextButton("assets/textures/MainMenu/UI/Button.png",
                                       {.x = 100, .y = 100}),
        m_increaseCountButton("assets/textures/MainMenu/UI/Button.png",
                              {.x = 500, .y = 500}),
        m_decreaseCountButton("assets/textures/MainMenu/UI/Button.png",
                              {.x = 1000, .y = 300}),
        m_startButton("assets/textures/MainMenu/UI/Button.png",
                      {.x = 800, .y = 600}) {}
  ~PlayerSelectState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  struct CustomInstrumentComposition {
    std::string name;
    std::variant<InstrumentComposition<InstrumentType::Custom_1>,
                 InstrumentComposition<InstrumentType::Custom_2>>
        composition;
  };
  std::vector<CustomInstrumentComposition> m_customInstruments;

  Button m_backNextButton;
  Button m_increaseCountButton;
  Button m_decreaseCountButton;
  Button m_startButton;

  std::inplace_vector<std::unique_ptr<PlayerBase>, 4> m_players;
  std::inplace_vector<std::uint8_t, 4> m_playerChoices;

  bool m_choosingCount{true};
  char m_playerCount{'1'};
};

} // namespace bh
