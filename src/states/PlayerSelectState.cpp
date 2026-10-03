#include <cstdint>
#include <charconv>
#include <variant>

#include "states/PlayerSelectState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"
#include "config/CustomInstrumentStore.hpp"
#include "ui/settings/Controls.hpp"

#include "gameplay/instruments/Custom.hpp"
#include "gameplay/instruments/Guitar.hpp"
#include "gameplay/instruments/Instrument.hpp"

#include "states/GameState.hpp"
#include "states/MainMenuState.hpp"

#include "ui/Button.hpp"

namespace bh {

void PlayerSelectState::draw() const noexcept {
  m_backButton.draw<WHITE, true, 40, TextAlign::Center>();

  switch (m_stage) {
  case Stage::ChoosingPlayerCount: {

    m_nextButton.draw<WHITE, true>();

    m_increaseCountButton.draw<WHITE, true, 40, TextAlign::Center>();
    m_decreaseCountButton.draw<WHITE, true, 40, TextAlign::Center>();

    const char playerCountText[2]{m_playerCount, '\0'};
    ResourceManager::drawText<Fonts::Type::Default>(
        playerCountText, Vector2{100.f, 350.f}, 64, theme::Text);
    break;
  }
  case Stage::ChoosingInstruments: {
    m_nextButton.draw<WHITE, true>();
    for (std::size_t i = 0; i < m_playerChoices.size(); ++i) {
      const auto choice = m_playerChoices[i];
      const int instrument = choice >> 1;
      const float y = 300 + i * 150.f;
      const std::string name = instrument == 0 ? "Guitar" : instrument == 1 ? "Bass" :
          instrument == 2 ? "Drums" : m_customInstruments[instrument - 3].name;
      settings_ui::text("Player " + std::to_string(i + 1), {250, y + 15}, 32);
      settings_ui::choice({500, y, 70, 70}, "<");
      ResourceManager::drawText<Fonts_t::Buttons>(name, {610, y + 15},
          std::min(32.f, 620.f * 32.f / std::max(1.f, ResourceManager::measureText<Fonts_t::Buttons>(name, 32))), theme::Text);
      settings_ui::choice({1260, y, 70, 70}, ">");
      settings_ui::choice({1400, y, 260, 70}, choice & 1 ? "Easy" : "Hard");
    }
    break;
  }
  case Stage::TestingInstruments: {
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

        if (buffer) {

          m_check[index].push_back(buffer);
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
        }
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
        m_stack.reset<GameState<1>>(move.operator()<1>(m_players));
        break;
      }
      case '2': {
        m_stack.reset<GameState<2>>(move.operator()<2>(m_players));
        break;
      }
      case '3': {
        m_stack.reset<GameState<3>>(move.operator()<3>(m_players));
        break;
      }
      case '4': {
        m_stack.reset<GameState<4>>(move.operator()<4>(m_players));
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

  const Vector2 mousePos = GetMousePosition();

  const bool choosingCount = m_stage == Stage::ChoosingPlayerCount;
  const bool testing = m_stage == Stage::TestingInstruments;

  const bool nextClicked = m_nextButton.updateInput(mousePos, !testing);
  const bool increaseClicked =
      m_increaseCountButton.updateInput(mousePos, choosingCount);
  const bool decreaseClicked =
      m_decreaseCountButton.updateInput(mousePos, choosingCount);
  const bool backClicked = m_backButton.updateInput(mousePos);

  if (m_stage == Stage::ChoosingInstruments) {
    const int count = 3 + static_cast<int>(m_customInstruments.size());
    for (std::size_t i = 0; i < m_playerChoices.size(); ++i) {
      const float y = 300 + i * 150.f;
      auto &choice = m_playerChoices[i];
      const int step = settings_ui::clicked({500, y, 70, 70}) ? -1 :
                       settings_ui::clicked({1260, y, 70, 70}) ? 1 : 0;
      if (step) choice = static_cast<std::uint8_t>((((choice >> 1) + step + count) % count) * 2 + (choice & 1));
      if (settings_ui::clicked({1400, y, 260, 70})) choice ^= 1;
    }
  }

  if (backClicked) [[unlikely]] {

    switch (m_stage) {

    case Stage::ChoosingPlayerCount: {
      m_stack.pop();
      break;
    }
    case Stage::ChoosingInstruments: {
      m_stage = Stage::ChoosingPlayerCount;
      m_nextButton.changeText("Choose Instruments");

      break;
    }
    case Stage::TestingInstruments: {
      m_stage = Stage::ChoosingInstruments;
      m_nextButton.changeText("Start");

      break;
    }
    default: {
      break;
    }
    }

  } else if (m_stage == Stage::ChoosingPlayerCount && increaseClicked)
      [[unlikely]] {

    switch (m_stage) {
    case Stage::ChoosingPlayerCount: {

      const bool condition{(++m_playerCount) <= '4'};
      m_playerCount = (m_playerCount * condition) + ('1' * !condition);
      break;
    }
    default: {
      break;
    }
    }

  } else if (m_stage == Stage::ChoosingPlayerCount && decreaseClicked) {

    switch (m_stage) {
    case Stage::ChoosingPlayerCount: {

      const bool condition{(--m_playerCount) >= '1'};
      m_playerCount = (m_playerCount * condition) + ('4' * !condition);
      break;
    }
    default: {
      break;
    }
    }

  } else if (m_stage != Stage::TestingInstruments && nextClicked) [[unlikely]] {

    switch (m_stage) {
    case Stage::ChoosingInstruments: {

      for (std::uint8_t i{}; i < m_playerChoices.size(); ++i) {
        const auto &playerChoice = m_playerChoices[i];

        switch (playerChoice >> 1) {

        case 0: {

          if (playerChoice & 1) {
            m_players[i] = std::make_unique<
                Player<InstrumentType::Guitar, Difficulty::Easy>>(i,
                                                                  m_songName);
          } else {
            m_players[i] = std::make_unique<
                Player<InstrumentType::Guitar, Difficulty::Hard>>(i,
                                                                  m_songName);
          }
          break;
        }

        case 1: {

          if (playerChoice & 1) {
            m_players[i] = std::make_unique<
                Player<InstrumentType::Bass, Difficulty::Easy>>(i, m_songName);
          } else {
            m_players[i] = std::make_unique<
                Player<InstrumentType::Bass, Difficulty::Hard>>(i, m_songName);
          }
          break;
        }

        case 2: {

          if (playerChoice & 1) {
            m_players[i] = std::make_unique<
                Player<InstrumentType::Drums, Difficulty::Easy>>(i, m_songName);
          } else {
            m_players[i] = std::make_unique<
                Player<InstrumentType::Drums, Difficulty::Hard>>(i, m_songName);
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
              // Holds Custom_1

              m_players[i] = std::make_unique<
                  Player<InstrumentType::Custom_1, Difficulty::Easy>>(
                  i, m_songName, customInstrument.name,
                  std::get<InstrumentComposition<InstrumentType::Custom_1>>(
                      customComposition));

            } else if (std::holds_alternative<
                           InstrumentComposition<InstrumentType::Custom_2>>(
                           customComposition)) {
              // Holds Custom_2

              m_players[i] = std::make_unique<
                  Player<InstrumentType::Custom_2, Difficulty::Easy>>(
                  i, m_songName, customInstrument.name,
                  std::get<InstrumentComposition<InstrumentType::Custom_2>>(
                      customComposition));

            } else {
              // Holds Custom_3

              m_players[i] = std::make_unique<
                  Player<InstrumentType::Custom_3, Difficulty::Easy>>(
                  i, m_songName, customInstrument.name,
                  std::get<InstrumentComposition<InstrumentType::Custom_3>>(
                      customComposition));
            }

          } else {

            if (std::holds_alternative<
                    InstrumentComposition<InstrumentType::Custom_1>>(
                    customComposition)) {
              // Holds Custom 1

              m_players[i] = std::make_unique<
                  Player<InstrumentType::Custom_1, Difficulty::Hard>>(
                  i, m_songName, customInstrument.name,
                  std::get<InstrumentComposition<InstrumentType::Custom_1>>(
                      customComposition));

            } else if (std::holds_alternative<
                           InstrumentComposition<InstrumentType::Custom_2>>(
                           customComposition)) {
              // Holds Custom 2

              m_players[i] = std::make_unique<
                  Player<InstrumentType::Custom_2, Difficulty::Hard>>(
                  i, m_songName, customInstrument.name,
                  std::get<InstrumentComposition<InstrumentType::Custom_2>>(
                      customComposition));

            } else {
              // Holds Custom 3

              m_players[i] = std::make_unique<
                  Player<InstrumentType::Custom_3, Difficulty::Hard>>(
                  i, m_songName, customInstrument.name,
                  std::get<InstrumentComposition<InstrumentType::Custom_3>>(
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
      m_counter = 0;

      m_check.clear();
      m_minMax.clear();

      for (std::uint8_t i{}; i < (m_playerCount - '0'); ++i) {
        m_check.emplace_back();
        m_minMax.emplace_back();
      }

      break;
    }
    case Stage::ChoosingPlayerCount: {

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

      m_stage = Stage::ChoosingInstruments;

      m_nextButton.changeText("Start");

      break;
    }
    default: {
      break;
    }
    }
  }
}

void PlayerSelectState::onEnter() noexcept {
  ResourceManager::loadTextures<Textures::UI>();
  ResourceManager::loadTexture<Textures::Settings::Controls>();

  m_customInstruments.clear();
  for (const auto &definition : CustomInstrumentStore::list()) {
    if (!definition.error.empty() || m_customInstruments.size() >= 125) continue;
    CustomInstrumentComposition instrument;
    instrument.name = definition.name;
    if (definition.kind == CustomKind::Sections) {
      instrument.composition = InstrumentComposition<InstrumentType::Custom_1>{
          static_cast<std::uint8_t>(definition.first), static_cast<std::uint8_t>(definition.second)};
    } else if (definition.kind == CustomKind::Bits) {
      instrument.composition = InstrumentComposition<InstrumentType::Custom_2>{
          static_cast<std::uint8_t>(definition.first), static_cast<std::uint8_t>(definition.second)};
    } else {
      const std::string stem = definition.path.stem().string();
      unsigned int number{};
      const auto [end, error] = std::from_chars(stem.data(), stem.data() + stem.size(), number);
      if (error != std::errc{} || end != stem.data() + stem.size() || number == 0 || number > 65535) continue;
      instrument.composition = InstrumentComposition<InstrumentType::Custom_3>{static_cast<std::uint16_t>(number)};
    }
    m_customInstruments.push_back(std::move(instrument));
  }
  for (auto &choice : m_playerChoices) {
    if ((choice >> 1) >= 3 + m_customInstruments.size()) choice = 1;
  }

  m_serial.openDevice(Settings::getSerialPort().c_str(),
                      Settings::getSerialBaudRate());
}

void PlayerSelectState::onExit() noexcept {
  ResourceManager::unloadTextures<Textures::UI>();
  ResourceManager::unloadTexture<Textures::Settings::Controls>();

  m_serial.closeDevice();

  m_backButton.resetInteraction();
  m_increaseCountButton.resetInteraction();
  m_decreaseCountButton.resetInteraction();
  m_nextButton.resetInteraction();
}

} // namespace bh
