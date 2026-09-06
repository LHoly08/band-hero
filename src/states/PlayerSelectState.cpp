#include <variant>

#include "states/PlayerSelectState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "gameplay/instruments/Custom.hpp"
#include "gameplay/instruments/Guitar.hpp"
#include "gameplay/instruments/Instrument.hpp"

#include "states/GameState.hpp"

namespace bh {

void PlayerSelectState::draw() const noexcept {
  m_backNextButton.draw();
  m_increaseCountButton.draw();
  m_decreaseCountButton.draw();

  DrawText(&m_playerCount, 100, 200, 21, BLACK);
}

void PlayerSelectState::update(float dt) noexcept {}

void PlayerSelectState::events() noexcept {
  if (IsMouseButtonPressed(0)) {
    const Vector2 MousePos{GetMousePosition()};

    if (m_backNextButton.pressed(MousePos)) {

      m_choosingCount = !m_choosingCount;

      const std::uint8_t loopTimes = (m_playerCount - '1') + 1;

      m_players.clear();
      for (std::uint8_t i{}; i < loopTimes; ++i) {
        m_players.emplace_back(nullptr);
      }

      m_playerChoices.clear();
      for (std::uint8_t i{}; i < loopTimes; ++i) {
        m_playerChoices.emplace_back(1);
      }

    } else if (m_increaseCountButton.pressed(MousePos)) {

      const bool condition{(++m_playerCount) <= '4'};
      m_playerCount = (m_playerCount * condition) + ('1' * !condition);

    } else if (m_decreaseCountButton.pressed(MousePos)) {

      const bool condition{(--m_playerCount) >= '1'};
      m_playerCount = (m_playerCount * condition) + ('4' * !condition);

    } else if (m_startButton.pressed(MousePos)) {

      for (std::uint8_t i{}; i < m_playerChoices.size(); ++i) {
        const auto &playerChoice = m_playerChoices[i];

        switch (playerChoice >> 1) {
        case 0: {
          if (playerChoice & 1) {
            m_players[i] = std::make_unique<
                Player<InstrumentType::Guitar, Difficulty::Easy>>(i);
          } else {
            m_players[i] = std::make_unique<
                Player<InstrumentType::Guitar, Difficulty::Hard>>(i);
          }
          break;
        }

        case 1: {
          if (playerChoice & 1) {
            m_players[i] = std::make_unique<
                Player<InstrumentType::Bass, Difficulty::Easy>>(i);
          } else {
            m_players[i] = std::make_unique<
                Player<InstrumentType::Bass, Difficulty::Hard>>(i);
          }
          break;
        }

        case 2: {
          if (playerChoice & 1) {
            m_players[i] = std::make_unique<
                Player<InstrumentType::Drums, Difficulty::Easy>>(i);
          } else {
            m_players[i] = std::make_unique<
                Player<InstrumentType::Drums, Difficulty::Hard>>(i);
          }
          break;
        }

        default: {
          const auto &customInstrument =
              m_customInstruments.at((playerChoice >> 1) - 3);
          const auto &customComposition = customInstrument.composition;

          if (playerChoice & 1) {

            if (std::holds_alternative<
                    InstrumentComposition<InstrumentType::Custom_1>>(
                    customComposition)) {

              m_players[i] = std::make_unique<
                  Player<InstrumentType::Custom_1, Difficulty::Easy>>(
                  i, customInstrument.name,
                  std::get<InstrumentComposition<InstrumentType::Custom_1>>(
                      customComposition));
            } else {

              m_players[i] = std::make_unique<
                  Player<InstrumentType::Custom_2, Difficulty::Easy>>(
                  i, customInstrument.name,
                  std::get<InstrumentComposition<InstrumentType::Custom_2>>(
                      customComposition));
            }

          } else {

            if (std::holds_alternative<
                    InstrumentComposition<InstrumentType::Custom_1>>(
                    customComposition)) {

              m_players[i] = std::make_unique<
                  Player<InstrumentType::Custom_1, Difficulty::Hard>>(
                  i, customInstrument.name,
                  std::get<InstrumentComposition<InstrumentType::Custom_1>>(
                      customComposition));

            } else {

              m_players[i] = std::make_unique<
                  Player<InstrumentType::Custom_2, Difficulty::Hard>>(
                  i, customInstrument.name,
                  std::get<InstrumentComposition<InstrumentType::Custom_2>>(
                      customComposition));
            }
          }
          break;
        }
        }
      }

      for (auto &player : m_players) {
        player->setPlayerCount((m_playerCount - '1') + 1);
      }

      switch (m_playerCount) {
      case '1': {

        m_stack.push<GameState<1>>(std::array<std::unique_ptr<PlayerBase>, 1>{
            std::move(m_players[0])});
        break;
      }
      case '2': {
        m_stack.push<GameState<2>>(std::array<std::unique_ptr<PlayerBase>, 2>{
            std::move(m_players[0]), std::move(m_players[1])});
        break;
      }
      case '3': {
        m_stack.push<GameState<3>>(std::array<std::unique_ptr<PlayerBase>, 3>{
            std::move(m_players[0]), std::move(m_players[1]),
            std::move(m_players[2])});
        break;
      }
      case '4': {
        m_stack.push<GameState<4>>(std::array<std::unique_ptr<PlayerBase>, 4>{
            std::move(m_players[0]), std::move(m_players[1]),
            std::move(m_players[2]), std::move(m_players[3])});
        break;
      }
      }
      m_players.clear();
    }
  }
}

void PlayerSelectState::onEnter() noexcept {}

void PlayerSelectState::onExit() noexcept {}

} // namespace bh
