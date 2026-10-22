#include <bit>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <variant>

#include "states/PlayerSelectState.hpp"

#include "raylib.h"

#include "config/CustomInstrumentStore.hpp"

#include "core/StateStack.hpp"

#include "gameplay/instruments/Custom.hpp"
#include "gameplay/instruments/Guitar.hpp"
#include "gameplay/instruments/Instrument.hpp"

#include "states/GameState.hpp"
#include "states/MainMenuState.hpp"

#include "ui/Button.hpp"

#include "ui/settings/Controls.hpp"

namespace bh {
namespace {
constexpr ::Rectangle DecreaseCount{760, 480, 90, 80};
constexpr ::Rectangle IncreaseCount{1070, 480, 90, 80};

struct InstrumentCardLayout {
  ::Rectangle panel;
  ::Rectangle previous;
  ::Rectangle next;
  ::Rectangle difficulty;
};

// Draw and input share the same layout, centered for one through four players.
InstrumentCardLayout instrumentCard(std::size_t index, std::size_t count) {
  const float totalWidth = count * 340.f + (count - 1) * 30.f;
  const float x = (1920 - totalWidth) / 2 + index * 370.f;
  return {{x, 280, 340, 590},
          {x + 55, 580, 100, 60},
          {x + 185, 580, 100, 60},
          {x + 40, 750, 260, 70}};
}
} // namespace

void bh::PlayerSelectState::draw() const noexcept {
  m_background.draw();

  switch (m_stage) {
  case Stage::ChoosingPlayerCount: {

    bh::settings_ui::card({620, 300, 680, 480});
    bh::settings_ui::text("PLAYER SETUP", {670, 335}, 21, bh::theme::Primary);
    bh::settings_ui::text("How many players?", {670, 375}, 44);
    bh::settings_ui::text("Choose 1-4 players to join the band.", {670, 435}, 25,
         bh::theme::MutedText);
    bh::settings_ui::choice(DecreaseCount, "<");
    bh::settings_ui::choice(IncreaseCount, ">");
    const std::string count(1, m_playerCount);
    const float countWidth =
        bh::ResourceManager::measureText<bh::Fonts_t::Buttons>(count, 90);
    bh::settings_ui::text(count, {960 - countWidth / 2, 475}, 90, bh::theme::Highlight);
    const std::string caption = m_playerCount == '1'   ? "Solo"
                                : m_playerCount == '2' ? "Duo"
                                : m_playerCount == '3' ? "Trio"
                                                       : "Squad";
    const float captionWidth =
        bh::ResourceManager::measureText<bh::Fonts_t::Buttons>(caption, 26);
    bh::settings_ui::text(caption, {960 - captionWidth / 2, 585}, 26, bh::theme::MutedText);
    m_nextButton.draw<WHITE, true, 30, bh::TextAlign::Center>();

    break;
  }
  case Stage::ChoosingInstruments: {

    const std::string heading = "Choose instruments";
    const float headingWidth =
        bh::ResourceManager::measureText<bh::Fonts_t::Buttons>(heading, 48);
    bh::settings_ui::text(heading, {960 - headingWidth / 2, 140}, 48);
    bh::settings_ui::text("Pick an instrument and difficulty for each player.", {620, 210}, 26,
         bh::theme::MutedText);
    m_nextButton.draw<WHITE, true, 40, bh::TextAlign::Center>();

    for (std::size_t i = 0; i < m_playerChoices.size(); ++i) {
      const auto choice = m_playerChoices[i];
      const int instrument = choice >> 1;
      const auto layout = instrumentCard(i, m_playerChoices.size());
      const float x = layout.panel.x;
      const std::string name = instrument == 0   ? "Guitar"
                               : instrument == 1 ? "Bass"
                               : instrument == 2
                                   ? "Drums"
                                   : m_customInstruments[instrument - 3].name;

      bh::settings_ui::card(layout.panel);
      bh::settings_ui::text("Player " + std::to_string(i + 1), {x + 32, 315}, 32,
           bh::theme::Primary);
      bh::settings_ui::text("INSTRUMENT", {x + 32, 365}, 20, bh::theme::MutedText);
      const float nameWidth =
          bh::ResourceManager::measureText<bh::Fonts_t::Buttons>(name, 32);
      const float nameSize =
          std::min(32.f, 280.f * 32.f / std::max(1.f, nameWidth));
      const float fittedWidth =
          bh::ResourceManager::measureText<bh::Fonts_t::Buttons>(name, nameSize);
      bh::settings_ui::text(name, {x + (340 - fittedWidth) / 2, 510}, nameSize, bh::theme::Text);
      bh::settings_ui::choice(layout.previous, "<");
      bh::settings_ui::choice(layout.next, ">");
      bh::settings_ui::text("DIFFICULTY", {x + 32, 710}, 20, bh::theme::MutedText);
      bh::settings_ui::choice(layout.difficulty, choice & 1 ? "Easy" : "Hard");
    }
    break;
  }
  case Stage::TestingInstruments: {

    const auto count = static_cast<std::size_t>(m_playerCount - '0');
    const bool ready = std::ranges::all_of(
        m_testNotes | std::views::take(count),
        [](const auto &notes) { return notes.size() == 3; });
    const int stage = !m_serial.isDeviceOpen() ? 0 : ready ? 2 : 1;
    unsigned int completedSteps{};
    for (std::size_t i = 0; i < count; ++i)
      completedSteps += m_testNotes[i].size();
    bh::settings_ui::card({400, 130, 1120, 780});
    bh::settings_ui::text("SOUNDCHECK", {460, 175}, 21, bh::theme::Primary);
    bh::settings_ui::text("Test your instruments", {460, 220}, 48);
    bh::settings_ui::text("A quick check before the band takes the stage.", {460, 285}, 25,
         bh::theme::MutedText);
    constexpr const char *labels[]{"1  Connect", "2  Test inputs", "3  Ready"};
    for (int i = 0; i < 3; ++i) {
      const float x = 460 + i * 340.f;
      bh::settings_ui::panel({x, 345, 320, 12}, i <= stage ? bh::theme::Primary : bh::theme::Border);
      const float progress =
          i < stage                  ? 1.f
          : i == stage && ready      ? std::clamp(m_counter / 3.f, 0.f, 1.f)
          : i == stage && stage == 1 ? completedSteps / (3.f * count)
                                     : 0.f;
      if (progress > 0)
        bh::settings_ui::panel({x, 345, 320 * progress, 12}, bh::theme::Highlight);
      bh::settings_ui::text(labels[i], {x, 380}, 28,
           i <= stage ? bh::theme::Text : bh::theme::MutedText);
    }
    if (!m_serial.isDeviceOpen()) {
      bh::settings_ui::text(bh::Settings::getSerialPort().empty()
               ? "No controller port selected"
               : "Could not open the controller port",
           {460, 445}, 28, bh::theme::Error);
      bh::settings_ui::text("Connect your controller, then retry. Choose its port in Settings.",
           {460, 490}, 23, bh::theme::MutedText);
      m_nextButton.draw<WHITE, true, 30, bh::TextAlign::Center>();
    } else {
      bh::settings_ui::text(ready ? "All instruments passed"
                 : "Play three distinct notes on each instrument.",
           {460, 445}, 28, ready ? bh::theme::Highlight : bh::theme::Text);
      constexpr const char *prompts[]{"Play the first note",
                                      "Play a different note",
                                      "Play a third distinct note", "Passed"};
      constexpr const char *steps[]{"Note 1", "Note 2", "Note 3"};
      for (std::size_t i = 0; i < count; ++i) {
        const float y = 515 + i * 76.f;
        const int step = static_cast<int>(m_testNotes[i].size());
        const int instrument = m_playerChoices[i] >> 1;
        const std::string name = instrument == 0   ? "Guitar"
                                 : instrument == 1 ? "Bass"
                                 : instrument == 2
                                     ? "Drums"
                                     : m_customInstruments[instrument - 3].name;
        bh::settings_ui::fittedText("P" + std::to_string(i + 1) + "  " + name, {460, y, 310, 28},
                   26, bh::theme::Text);
        bh::settings_ui::text(prompts[step], {460, y + 32}, 20,
             step == 3 ? bh::theme::Highlight : bh::theme::MutedText);
        for (int j = 0; j < 3; ++j) {
          const float x = 830 + j * 205.f;
          bh::settings_ui::panel({x, y + 4, 185, 8},
                j < step ? bh::theme::Highlight : bh::theme::Border);
          bh::settings_ui::text(steps[j], {x, y + 25}, 22,
               j < step    ? bh::theme::Highlight
               : j == step ? bh::theme::Text
                           : bh::theme::Disabled);
        }
      }
      if (ready)
        bh::settings_ui::text("Starting in " +
                 std::to_string(std::max(1, 3 - static_cast<int>(m_counter))) +
                 "...",
             {460, 840}, 25, bh::theme::Highlight);
    }
    break;
  }
  }

  m_backButton.draw<WHITE, true, 40, bh::TextAlign::Center>();
}

void bh::PlayerSelectState::update(float dt) noexcept {
  m_background.update(dt);

  switch (m_stage) {
  case Stage::TestingInstruments: {

    if (!m_serial.isDeviceOpen()) {
      m_nextButton.changeText("Retry connection");
      m_nextButton.setPosition({780, 790});
      m_testNotes = {};
      m_counter = 0;
      break;
    }

    std::uint32_t packet{};
    const int received = m_serial.readBytes(&packet, sizeof(packet), 1);
    if (received == sizeof(packet)) {
      if constexpr (std::endian::native == std::endian::big) {
        packet = std::byteswap(packet);
      }
      const auto index = packet & 0b11;
      const auto state = packet >> 2;
      if (index < static_cast<unsigned int>(m_playerCount - '0')) {
        const auto choice = m_playerChoices[index] >> 1;
        unsigned sections{};
        unsigned bits{};
        if (choice == 0 || choice == 1) {
          sections = choice == 0 ? 6 : 4;
          bits = 5;
        } else if (choice >= 3) {
          const auto &composition = m_customInstruments[choice - 3].composition;
          if (const auto *layout = std::get_if<
                  bh::InstrumentComposition<bh::InstrumentType::Custom_1>>(&composition)) {
            sections = layout->NumberSections;
            bits = layout->NumberBitsSection;
          }
        }
        const auto presses = m_controllerStates[index].update(state, sections, bits);
        auto &notes = m_testNotes[index];
        if (presses != 0 && notes.size() < notes.capacity() &&
            !std::ranges::contains(notes, state)) {
          notes.push_back(state);
        }
      }
    }
    const bool ready = std::ranges::all_of(
        m_testNotes | std::views::take(m_playerCount - '0'),
        [](const auto &notes) { return notes.size() == 3; });
    m_counter = ready ? m_counter + dt : 0.f;

    if (m_counter >= 3.f) {

      auto move = []<std::size_t N>(auto &v) {
        std::array<std::unique_ptr<bh::PlayerBase>, N> players;

        std::ranges::transform(v | std::views::take(N), players.begin(),
                               [](auto &ptr) { return std::move(ptr); });

        return players;
      };

      std::string audioDirectory = m_setList.getAudio();
      m_setList.pop();

      switch (m_playerCount) {
      case '1': {
        m_stack.reset<bh::GameState<1>>(move.operator()<1>(m_players),
                                   std::move(audioDirectory),
                                   std::move(m_setList));
        break;
      }
      case '2': {
        m_stack.reset<bh::GameState<2>>(move.operator()<2>(m_players),
                                   std::move(audioDirectory),
                                   std::move(m_setList));
        break;
      }
      case '3': {
        m_stack.reset<bh::GameState<3>>(move.operator()<3>(m_players),
                                   std::move(audioDirectory),
                                   std::move(m_setList));
        break;
      }
      case '4': {
        m_stack.reset<bh::GameState<4>>(move.operator()<4>(m_players),
                                   std::move(audioDirectory),
                                   std::move(m_setList));
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

void bh::PlayerSelectState::events() noexcept {

  const ::Vector2 mousePos = ::GetMousePosition();

  const bool choosingCount = m_stage == Stage::ChoosingPlayerCount;
  const bool testing = m_stage == Stage::TestingInstruments;

  const bool nextClicked =
      m_nextButton.updateInput(mousePos, !testing || !m_serial.isDeviceOpen());
  const bool increaseClicked =
      choosingCount && bh::settings_ui::clicked(IncreaseCount);
  const bool decreaseClicked =
      choosingCount && bh::settings_ui::clicked(DecreaseCount);
  const bool backClicked = m_backButton.updateInput(mousePos);

  if (m_stage == Stage::ChoosingInstruments) {
    const int count = 3 + static_cast<int>(m_customInstruments.size());
    for (std::size_t i = 0; i < m_playerChoices.size(); ++i) {
      const auto layout = instrumentCard(i, m_playerChoices.size());
      auto &choice = m_playerChoices[i];
      const int step = bh::settings_ui::clicked(layout.previous) ? -1
                       : bh::settings_ui::clicked(layout.next)   ? 1
                                                             : 0;
      if (step)
        choice = static_cast<std::uint8_t>(
            (((choice >> 1) + step + count) % count) * 2 + (choice & 1));
      if (bh::settings_ui::clicked(layout.difficulty))
        choice ^= 1;
    }
  }

  if (testing && !m_serial.isDeviceOpen() && nextClicked && !backClicked) {
    m_serial.closeDevice();
    if (!bh::Settings::getSerialPort().empty() &&
        m_serial.openDevice(bh::Settings::getSerialPort().c_str(),
                            bh::Settings::getSerialBaudRate()) != 1) {
      m_serial.closeDevice();
    }
    if (m_serial.isDeviceOpen()) {
      m_serial.flushReceiver();
      m_controllerStates = {};
    }
    return;
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
      m_nextButton.setPosition({780, 650});

      break;
    }
    case Stage::TestingInstruments: {
      m_stage = Stage::ChoosingInstruments;
      m_nextButton.setPosition({780, 925});
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

      std::string songName = m_setList.getCharts();

      for (std::uint8_t i{}; i < m_playerChoices.size(); ++i) {
        const auto &playerChoice = m_playerChoices[i];

        switch (playerChoice >> 1) {

        case 0: {

          if (playerChoice & 1) {
            m_players[i] = std::make_unique<
                bh::Player<bh::InstrumentType::Guitar, bh::Difficulty::Easy>>(i, songName);
          } else {
            m_players[i] = std::make_unique<
                bh::Player<bh::InstrumentType::Guitar, bh::Difficulty::Hard>>(i, songName);
          }
          break;
        }

        case 1: {

          if (playerChoice & 1) {
            m_players[i] = std::make_unique<
                bh::Player<bh::InstrumentType::Bass, bh::Difficulty::Easy>>(i, songName);
          } else {
            m_players[i] = std::make_unique<
                bh::Player<bh::InstrumentType::Bass, bh::Difficulty::Hard>>(i, songName);
          }
          break;
        }

        case 2: {

          if (playerChoice & 1) {
            m_players[i] = std::make_unique<
                bh::Player<bh::InstrumentType::Drums, bh::Difficulty::Easy>>(i, songName);
          } else {
            m_players[i] = std::make_unique<
                bh::Player<bh::InstrumentType::Drums, bh::Difficulty::Hard>>(i, songName);
          }
          break;
        }

        default: {

          const auto &customInstrument =
              m_customInstruments.at((playerChoice >> 1) - 3);
          const auto &customComposition = customInstrument.composition;

          if (playerChoice & 1) {

            if (std::holds_alternative<
                    bh::InstrumentComposition<bh::InstrumentType::Custom_1>>(
                    customComposition)) {
              // Holds Custom_1

              m_players[i] = std::make_unique<
                  bh::Player<bh::InstrumentType::Custom_1, bh::Difficulty::Easy>>(
                  i, songName, customInstrument.name,
                  std::get<bh::InstrumentComposition<bh::InstrumentType::Custom_1>>(
                      customComposition));

            } else if (std::holds_alternative<
                           bh::InstrumentComposition<bh::InstrumentType::Custom_2>>(
                           customComposition)) {
              // Holds Custom_2

              m_players[i] = std::make_unique<
                  bh::Player<bh::InstrumentType::Custom_2, bh::Difficulty::Easy>>(
                  i, songName, customInstrument.name,
                  std::get<bh::InstrumentComposition<bh::InstrumentType::Custom_2>>(
                      customComposition));

            } else {
              // Holds Custom_3

              m_players[i] = std::make_unique<
                  bh::Player<bh::InstrumentType::Custom_3, bh::Difficulty::Easy>>(
                  i, songName, customInstrument.name,
                  std::get<bh::InstrumentComposition<bh::InstrumentType::Custom_3>>(
                      customComposition));
            }

          } else {

            if (std::holds_alternative<
                    bh::InstrumentComposition<bh::InstrumentType::Custom_1>>(
                    customComposition)) {
              // Holds Custom 1

              m_players[i] = std::make_unique<
                  bh::Player<bh::InstrumentType::Custom_1, bh::Difficulty::Hard>>(
                  i, songName, customInstrument.name,
                  std::get<bh::InstrumentComposition<bh::InstrumentType::Custom_1>>(
                      customComposition));

            } else if (std::holds_alternative<
                           bh::InstrumentComposition<bh::InstrumentType::Custom_2>>(
                           customComposition)) {
              // Holds Custom 2

              m_players[i] = std::make_unique<
                  bh::Player<bh::InstrumentType::Custom_2, bh::Difficulty::Hard>>(
                  i, songName, customInstrument.name,
                  std::get<bh::InstrumentComposition<bh::InstrumentType::Custom_2>>(
                      customComposition));

            } else {
              // Holds Custom 3

              m_players[i] = std::make_unique<
                  bh::Player<bh::InstrumentType::Custom_3, bh::Difficulty::Hard>>(
                  i, songName, customInstrument.name,
                  std::get<bh::InstrumentComposition<bh::InstrumentType::Custom_3>>(
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

      m_testNotes = {};
      m_counter = 0;
      m_serial.closeDevice();
      if (!bh::Settings::getSerialPort().empty() &&
          m_serial.openDevice(bh::Settings::getSerialPort().c_str(),
                              bh::Settings::getSerialBaudRate()) != 1) {
        m_serial.closeDevice();
      }
      if (m_serial.isDeviceOpen()) {
        m_serial.flushReceiver();
        m_controllerStates = {};
      }
      m_nextButton.changeText("Retry connection");
      m_nextButton.setPosition({780, 790});

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
      m_nextButton.setPosition({780, 925});

      break;
    }
    default: {
      break;
    }
    }
  }
}

bh::PlayerSelectState::PlayerSelectState(bh::StateStack &stack,
                                     bh::SetList setList) noexcept
    : bh::State(stack), m_setList(std::move(setList)) {
  m_customInstruments.clear();
  for (const auto &definition : bh::CustomInstrumentStore::list()) {
    if (!definition.error.empty() || m_customInstruments.size() >= 125)
      continue;
    bh::CustomInstrumentComposition instrument;
    instrument.name = definition.name;
    if (definition.kind == bh::CustomKind::Sections) {
      instrument.composition = bh::InstrumentComposition<bh::InstrumentType::Custom_1>{
          static_cast<std::uint8_t>(definition.first),
          static_cast<std::uint8_t>(definition.second)};
    } else if (definition.kind == bh::CustomKind::Bits) {
      instrument.composition = bh::InstrumentComposition<bh::InstrumentType::Custom_2>{
          static_cast<std::uint8_t>(definition.first),
          static_cast<std::uint8_t>(definition.second)};
    } else {
      const std::string stem = definition.path.stem().string();
      unsigned int number{};
      const auto [end, error] =
          std::from_chars(stem.data(), stem.data() + stem.size(), number);
      if (error != std::errc{} || end != stem.data() + stem.size() ||
          number == 0 || number > 65535)
        continue;
      instrument.composition = bh::InstrumentComposition<bh::InstrumentType::Custom_3>{
          static_cast<std::uint16_t>(number)};
    }
    m_customInstruments.push_back(std::move(instrument));
  }
  for (auto &choice : m_playerChoices) {
    if ((choice >> 1) >= 3 + m_customInstruments.size())
      choice = 1;
  }
}

void bh::PlayerSelectState::onEnter() noexcept {
  if (!bh::Settings::getSerialPort().empty() &&
      m_serial.openDevice(bh::Settings::getSerialPort().c_str(),
                          bh::Settings::getSerialBaudRate()) != 1) {
    m_serial.closeDevice();
  }
}

void bh::PlayerSelectState::onExit() noexcept {
  m_serial.closeDevice();

  m_backButton.resetInteraction();
  m_nextButton.resetInteraction();
}

} // namespace bh
