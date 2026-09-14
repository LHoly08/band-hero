#include <cstdint>
#include <variant>

#include "states/PlayerSelectState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "gameplay/instruments/Custom.hpp"
#include "gameplay/instruments/Guitar.hpp"
#include "gameplay/instruments/Instrument.hpp"

#include "states/GameState.hpp"
#include "states/MainMenuState.hpp"

namespace bh {

void PlayerSelectState::draw() const noexcept {

  switch (m_stage) {
  case Stage::ChoosingPlayerCount: {
    m_backNextButton.draw<WHITE, true>();
    m_increaseCountButton.draw<WHITE, true>();
    m_decreaseCountButton.draw<WHITE, true>();
    m_mainMenuButton.draw<WHITE, true>();

    const char playerCountText[2]{m_playerCount, '\0'};
    ResourceManager::drawText<Fonts::Type::Default>(
        playerCountText, Vector2{100.f, 200.f}, 64, BLACK);
    break;
  }
  case Stage::ChoosingInstruments: {
    m_backNextButton.draw<WHITE, true>();
    m_startButton.draw<WHITE, true>();
    m_mainMenuButton.draw<WHITE, true>();

    break;
  }
  case Stage::TestingInstruments: {
    m_startButton.draw<WHITE, true>();
    break;
  }
  }
}

void PlayerSelectState::update(float dt) noexcept {

  switch (m_stage) {
  case Stage::TestingInstruments: {
    m_counter += dt;

    if (std::uint32_t buffer{};
        m_serial.readBytes(&buffer, sizeof(buffer), 1) == sizeof(buffer)) {

      if constexpr (std::endian::native == std::endian::big) {
        buffer = std::byteswap(buffer);
      }

      if (std::uint8_t index = buffer & 0b11; index < (m_playerCount - '0'))
          [[likely]] {
        if (m_check[index].size() == m_check[index].capacity()) [[likely]] {
          goto VectorAlreadyFull;
        }

        {
          std::uint8_t &max = m_minMax[index].first;
          const bool cond = buffer > m_check[index][max];
          max = index * cond + max * !cond;
        }
        {
          std::uint8_t &min = m_minMax[index].second;
          const bool cond = buffer < m_check[index][min];
          min = index * cond + min * !cond;
        }
        m_check[index].push_back(buffer);
      }
    }
    for (auto &&[vals, minMax] : std::ranges::views::zip(m_check, m_minMax)) {
      if (vals.size() == vals.capacity()) {
        const std::uint8_t max = minMax.first;
        const std::uint8_t min = minMax.second;

        std::uint32_t mid = vals[((0b11 ^ max) ^ min)];

        if (std::uint32_t result{(vals[max] - mid) + vals[min]};
            result != vals[max] && result != vals[min] && result != mid) {

          m_checksPassed |= 1 << (mid & 0b11);

        } else {

          m_checksPassed &= ~(1 << (mid & 0b11));
          m_check[mid & 0b11].clear();
          m_minMax[mid & 0b11] = {0, 0};
        }
        m_counter = 0;
      }
    }
  VectorAlreadyFull:

    if (m_counter >= 3 && m_checksPassed == ((1 << (m_playerCount - '0')) - 1))
        [[unlikely]] {
      auto move = []<std::size_t N>(auto &v) {
        std::array<std::unique_ptr<PlayerBase>, N> players;

        std::ranges::transform(v | std::views::take(N), players.begin(),
                               [](auto &ptr) { return std::move(ptr); });

        return players;
      };

      switch (m_playerCount) {
      case '1': {
        m_stack.push<GameState<1>>(move.operator()<1>(m_players));
        break;
      }
      case '2': {
        m_stack.push<GameState<2>>(move.operator()<2>(m_players));
        break;
      }
      case '3': {
        m_stack.push<GameState<3>>(move.operator()<3>(m_players));
        break;
      }
      case '4': {
        m_stack.push<GameState<4>>(move.operator()<4>(m_players));
        break;
      }
      }
      m_players.clear();
    }
    break;
  }
  default: {
    break;
  }
  }
}

void PlayerSelectState::events() noexcept {

  if (IsMouseButtonPressed(0)) [[unlikely]] {
    const Vector2 MousePos{GetMousePosition()};

    if (m_backNextButton.pressed(MousePos)) [[unlikely]] {

      switch (m_stage) {

      case Stage::ChoosingPlayerCount: {

        m_stage = Stage::ChoosingInstruments;

        m_backNextButton.changeText("Go Back");

        const std::uint8_t loopTimes = (m_playerCount - '0');

        // Every player initialized to nullptr
        m_players.clear();
        for (std::uint8_t i{}; i < loopTimes; ++i) {
          m_players.emplace_back(nullptr);
        }

        // Every player starts with Easy selected
        m_playerChoices.clear();
        for (std::uint8_t i{}; i < loopTimes; ++i) {
          m_playerChoices.emplace_back(1);
        }
        break;
      }
      case Stage::ChoosingInstruments: {
        m_stage = Stage::ChoosingPlayerCount;

        m_backNextButton.changeText("Choose Instruments");

        break;
      }
      default: {
        break;
      }
      }

    } else if (m_increaseCountButton.pressed(MousePos)) [[unlikely]] {

      switch (m_stage) {
      case Stage::ChoosingPlayerCount: {

        const bool condition{(++m_playerCount) <= '4'};
        m_playerCount = (m_playerCount * condition) + ('1' * !condition);
        break;
      }
      case Stage::ChoosingInstruments: {
        break;
      }
      default: {
        break;
      }
      }

    } else if (m_decreaseCountButton.pressed(MousePos)) {

      switch (m_stage) {
      case Stage::ChoosingPlayerCount: {

        const bool condition{(--m_playerCount) >= '1'};
        m_playerCount = (m_playerCount * condition) + ('4' * !condition);
        break;
      }
      case Stage::ChoosingInstruments: {
        break;
      }
      default: {
        break;
      }
      }

    } else if (m_startButton.pressed(MousePos)) [[unlikely]] {

      switch (m_stage) {
      case Stage::ChoosingInstruments: {

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
          player->setPlayerCount(m_playerCount - '0');
        }

        m_stage = Stage::TestingInstruments;

        m_checksPassed = 0;

        m_check.clear();
        m_minMax.clear();

        for (std::uint8_t i{}; i < (m_playerCount - '0'); ++i) {
          m_check.emplace_back();
          m_minMax.emplace_back();
        }

        m_startButton.changeText("Cancel");

        break;
      }
      case Stage::TestingInstruments: {

        m_stage = Stage::ChoosingInstruments;

        m_startButton.changeText("Start");

        break;
      }
      default: {
        break;
      }
      }
    } else if (m_mainMenuButton.pressed(MousePos)) {
      switch (m_stage) {
      case Stage::TestingInstruments: {
        break;
      }
      default: {
        m_stack.replace<MainMenuState>();
        break;
      }
      }
    }
  }
}

void PlayerSelectState::onEnter() noexcept {}

void PlayerSelectState::onExit() noexcept {}

} // namespace bh
