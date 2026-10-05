#include <cstdint>
#include <bit>
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
namespace {
constexpr Rectangle DecreaseCount{760, 480, 90, 80};
constexpr Rectangle IncreaseCount{1070, 480, 90, 80};

struct InstrumentCardLayout {
  Rectangle panel;
  Rectangle previous;
  Rectangle next;
  Rectangle difficulty;
};

// Draw and input share the same layout, centered for one through four players.
InstrumentCardLayout instrumentCard(std::size_t index, std::size_t count) {
  const float totalWidth = count * 340.f + (count - 1) * 30.f;
  const float x = (1920 - totalWidth) / 2 + index * 370.f;
  return {{x, 280, 340, 590}, {x + 55, 580, 100, 60},
          {x + 185, 580, 100, 60}, {x + 40, 750, 260, 70}};
}
}

void PlayerSelectState::draw() const noexcept {
  drawBackground();

  switch (m_stage) {
  case Stage::ChoosingPlayerCount: {
    using namespace settings_ui;
    card({620, 300, 680, 480});
    text("PLAYER SETUP", {670, 335}, 21, theme::Primary);
    text("How many players?", {670, 375}, 44);
    text("Choose 1-4 players to join the band.", {670, 435}, 25, theme::MutedText);
    choice(DecreaseCount, "<");
    choice(IncreaseCount, ">");
    const std::string count(1, m_playerCount);
    const float countWidth = ResourceManager::measureText<Fonts_t::Buttons>(count, 90);
    text(count, {960 - countWidth / 2, 475}, 90, theme::Highlight);
    const std::string caption = m_playerCount == '1' ? "Solo" : m_playerCount == '2' ? "Duo" : m_playerCount == '3' ? "Trio" : "Squad";
    const float captionWidth = ResourceManager::measureText<Fonts_t::Buttons>(caption, 26);
    text(caption, {960 - captionWidth / 2, 585}, 26, theme::MutedText);
    m_nextButton.draw<WHITE, true, 30, TextAlign::Center>();

    break;
  }
  case Stage::ChoosingInstruments: {
    using namespace settings_ui;
    const std::string heading = "Choose instruments";
    const float headingWidth = ResourceManager::measureText<Fonts_t::Buttons>(heading, 48);
    text(heading, {960 - headingWidth / 2, 140}, 48);
    text("Pick an instrument and difficulty for each player.", {620, 210}, 26, theme::MutedText);
    m_nextButton.draw<WHITE, true, 40, TextAlign::Center>();

    for (std::size_t i = 0; i < m_playerChoices.size(); ++i) {
      const auto choice = m_playerChoices[i];
      const int instrument = choice >> 1;
      const auto layout = instrumentCard(i, m_playerChoices.size());
      const float x = layout.panel.x;
      const std::string name = instrument == 0 ? "Guitar" : instrument == 1 ? "Bass" :
          instrument == 2 ? "Drums" : m_customInstruments[instrument - 3].name;

      card(layout.panel);
      text("Player " + std::to_string(i + 1), {x + 32, 315}, 32, theme::Primary);
      text("INSTRUMENT", {x + 32, 365}, 20, theme::MutedText);
      const float nameWidth = ResourceManager::measureText<Fonts_t::Buttons>(name, 32);
      const float nameSize = std::min(32.f, 280.f * 32.f / std::max(1.f, nameWidth));
      const float fittedWidth = ResourceManager::measureText<Fonts_t::Buttons>(name, nameSize);
      text(name, {x + (340 - fittedWidth) / 2, 510}, nameSize, theme::Text);
      settings_ui::choice(layout.previous, "<");
      settings_ui::choice(layout.next, ">");
      text("DIFFICULTY", {x + 32, 710}, 20, theme::MutedText);
      settings_ui::choice(layout.difficulty, choice & 1 ? "Easy" : "Hard");
    }
    break;
  }
  case Stage::TestingInstruments: {
    using namespace settings_ui;
    const auto count = static_cast<std::size_t>(m_playerCount - '0');
    const bool ready = std::ranges::all_of(
        m_testNotes | std::views::take(count), [](const auto &notes) { return notes.size() == 3; });
    const int stage = !m_controllerConnected ? 0 : ready ? 2 : 1;
    unsigned int completedSteps{};
    for (std::size_t i = 0; i < count; ++i) completedSteps += m_testNotes[i].size();
    card({400, 130, 1120, 780});
    text("SOUNDCHECK", {460, 175}, 21, theme::Primary);
    text("Test your instruments", {460, 220}, 48);
    text("A quick check before the band takes the stage.", {460, 285}, 25, theme::MutedText);
    constexpr const char *labels[]{"1  Connect", "2  Test inputs", "3  Ready"};
    for (int i = 0; i < 3; ++i) {
      const float x = 460 + i * 340.f;
      panel({x, 345, 320, 12}, i <= stage ? theme::Primary : theme::Border);
      const float progress = i < stage ? 1.f : i == stage && ready ?
          std::clamp(m_counter / 3.f, 0.f, 1.f) : i == stage && stage == 1 ?
          completedSteps / (3.f * count) : 0.f;
      if (progress > 0) panel({x, 345, 320 * progress, 12}, theme::Highlight);
      text(labels[i], {x, 380}, 28, i <= stage ? theme::Text : theme::MutedText);
    }
    if (!m_controllerConnected) {
      text(Settings::getSerialPort().empty() ? "No controller port selected" : "Could not open the controller port",
           {460, 445}, 28, theme::Error);
      text("Connect your controller, then retry. Choose its port in Settings.",
           {460, 490}, 23, theme::MutedText);
      m_nextButton.draw<WHITE, true, 30, TextAlign::Center>();
    } else {
      text(ready ? "All instruments passed" : "Play three distinct notes on each instrument.",
           {460, 445}, 28, ready ? theme::Highlight : theme::Text);
      constexpr const char *prompts[]{"Play the first note", "Play a different note", "Play a third distinct note", "Passed"};
      constexpr const char *steps[]{"Note 1", "Note 2", "Note 3"};
      for (std::size_t i = 0; i < count; ++i) {
        const float y = 515 + i * 76.f;
        const int step = static_cast<int>(m_testNotes[i].size());
        const int instrument = m_playerChoices[i] >> 1;
        const std::string name = instrument == 0 ? "Guitar" : instrument == 1 ? "Bass" :
            instrument == 2 ? "Drums" : m_customInstruments[instrument - 3].name;
        fittedText("P" + std::to_string(i + 1) + "  " + name,
                   {460, y, 310, 28}, 26, theme::Text);
        text(prompts[step], {460, y + 32}, 20, step == 3 ? theme::Highlight : theme::MutedText);
        for (int j = 0; j < 3; ++j) {
          const float x = 830 + j * 205.f;
          panel({x, y + 4, 185, 8}, j < step ? theme::Highlight : theme::Border);
          text(steps[j], {x, y + 25}, 22,
               j < step ? theme::Highlight : j == step ? theme::Text : theme::Disabled);
        }
      }
      if (ready) text("Starting in " + std::to_string(std::max(1, 3 - static_cast<int>(m_counter))) + "...",
                      {460, 840}, 25, theme::Highlight);
    }
    break;
  }
  }

  m_backButton.draw<WHITE, true, 40, TextAlign::Center>();
}

void PlayerSelectState::update(float dt) noexcept {

  switch (m_stage) {
  case Stage::TestingInstruments: {

    if (!m_controllerConnected) break;

    if (!m_serial.isDeviceOpen()) {
      m_controllerConnected = false;
      m_nextButton.changeText("Retry connection");
      m_nextButton.setPosition({780, 790});
      m_testNotes = {};
      m_counter = 0;
      break;
    }

    // A read must return all four bytes. Short reads are consumed and discarded,
    // never retained to complete a packet during a later update.
    std::array<unsigned char, 4> packet{};
    const int received = m_serial.readBytes(packet.data(), packet.size(), 1);
    
    if (received == static_cast<int>(packet.size())) {
      auto buffer = std::bit_cast<std::uint32_t>(packet);
      
      if constexpr (std::endian::native == std::endian::big) {
        buffer = std::byteswap(buffer);
      }
      const auto index = buffer & 0b11;
      if (index < static_cast<unsigned int>(m_playerCount - '0')) {
        auto &notes = m_testNotes[index];
        const auto note = buffer >> 2;

        if (note != 0 && notes.size() < notes.capacity() &&
            !std::ranges::contains(notes, note)) {
          notes.push_back(note);
        }
      }
    } else if (received < 0) {
      m_serial.closeDevice();
      m_controllerConnected = false;
      m_nextButton.changeText("Retry connection");
      m_nextButton.setPosition({780, 790});
      m_testNotes = {};
      m_counter = 0;
      break;
    }
    const bool ready = std::ranges::all_of(
        m_testNotes | std::views::take(m_playerCount - '0'),
        [](const auto &notes) { return notes.size() == 3; });
    m_counter = ready ? m_counter + dt : 0.f;
    
    if (m_counter >= 3.f) {

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

  const bool nextClicked = m_nextButton.updateInput(mousePos, !testing || !m_controllerConnected);
  const bool increaseClicked = choosingCount && settings_ui::clicked(IncreaseCount);
  const bool decreaseClicked = choosingCount && settings_ui::clicked(DecreaseCount);
  const bool backClicked = m_backButton.updateInput(mousePos);

  if (m_stage == Stage::ChoosingInstruments) {
    const int count = 3 + static_cast<int>(m_customInstruments.size());
    for (std::size_t i = 0; i < m_playerChoices.size(); ++i) {
      const auto layout = instrumentCard(i, m_playerChoices.size());
      auto &choice = m_playerChoices[i];
      const int step = settings_ui::clicked(layout.previous) ? -1 :
                       settings_ui::clicked(layout.next) ? 1 : 0;
      if (step) choice = static_cast<std::uint8_t>((((choice >> 1) + step + count) % count) * 2 + (choice & 1));
      if (settings_ui::clicked(layout.difficulty)) choice ^= 1;
    }
  }

  if (testing && !m_controllerConnected && nextClicked && !backClicked) {
    m_serial.closeDevice();
    m_controllerConnected = !Settings::getSerialPort().empty() &&
        m_serial.openDevice(Settings::getSerialPort().c_str(), Settings::getSerialBaudRate()) == 1;
    if (m_controllerConnected) m_serial.flushReceiver();
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

      m_testNotes = {};
      m_counter = 0;
      m_serial.closeDevice();
      m_controllerConnected = !Settings::getSerialPort().empty() &&
          m_serial.openDevice(Settings::getSerialPort().c_str(), Settings::getSerialBaudRate()) == 1;
      if (m_controllerConnected) m_serial.flushReceiver();
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

void PlayerSelectState::onEnter() noexcept {
  ResourceManager::loadTextures<Textures::UI>();

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

  m_serial.closeDevice();

  m_backButton.resetInteraction();
  m_nextButton.resetInteraction();
}

} // namespace bh
