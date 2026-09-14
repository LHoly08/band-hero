#pragma once

#include <bitset>
#include <filesystem>
#include <inplace_vector>
#include <memory>
#include <variant>

#include "gameplay/Player.hpp"

#include "gameplay/instruments/Custom.hpp"
#include "gameplay/instruments/Instrument.hpp"

#include "serial/serialib.h"

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class PlayerSelectState final : public State {
public:
  inline PlayerSelectState(StateStack &stack) noexcept
      : State(stack), m_backNextButton({.x = 100, .y = 100}, {0, 0, 500, 200},
                                       "Choose Instruments"),
        m_increaseCountButton({.x = 500, .y = 500}, {0, 0, 500, 200}, ">"),
        m_decreaseCountButton({.x = 1000, .y = 300}, {0, 0, 500, 200}, "<"),
        m_startButton({.x = 800, .y = 600}, {0, 0, 500, 200}, "Start"),
        m_mainMenuButton({.x = 1420, .y = 0}, {0, 0, 500, 200}, "Main Menu") {
    m_serial.openDevice(Settings::getSerialPort().c_str(),
                        Settings::getSerialBaudRate());
  }

  ~PlayerSelectState() override { m_serial.closeDevice(); }

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  serialib m_serial;

  Button m_backNextButton;
  Button m_increaseCountButton;
  Button m_decreaseCountButton;
  Button m_startButton;
  Button m_mainMenuButton;

  std::vector<CustomInstrumentComposition> m_customInstruments;

  std::inplace_vector<std::uint8_t, 4> m_playerChoices;
  std::inplace_vector<std::unique_ptr<PlayerBase>, 4> m_players;
  std::inplace_vector<std::inplace_vector<std::uint32_t, 3>, 4> m_check;
  std::inplace_vector<std::pair<std::uint8_t, std::uint8_t>, 4> m_minMax;

  float m_counter{};

  std::uint8_t m_checksPassed{};

  enum class Stage : std::uint8_t {
    ChoosingPlayerCount = 0,
    ChoosingInstruments,
    TestingInstruments,
  };
  Stage m_stage{Stage::ChoosingPlayerCount};
  char m_playerCount{'1'};
};

} // namespace bh
