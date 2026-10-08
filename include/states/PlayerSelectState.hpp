#pragma once

#include <array>
#include <filesystem>
#include <inplace_vector>
#include <memory>
#include <variant>

#include "core/ResourceManager.hpp"

#include "gameplay/Player.hpp"

#include "gameplay/instruments/Instrument.hpp"

#include "serial/serialib.h"

#include "states/State.hpp"
#include "ui/AnimatedBackground.hpp"

#include "ui/Button.hpp"

namespace bh {

class PlayerSelectState final : public State {
public:
  PlayerSelectState(StateStack &stack, std::string &&songName,
                    std::string &&audioDirectory) noexcept;
  ~PlayerSelectState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<ResourceManager::TextureHandle> m_textures =
      ResourceManager::acquireTextures<Textures::UI>();
  AnimatedBackground m_background;
  serialib m_serial;

  Button m_nextButton{
    {.x = 780, .y = 650},
    {0, 0, 360, 100},
    "Choose Instruments"
  };
  Button m_backButton{
    {.x = 30, .y = 950},
    {0, 640, 320, 96},
    "Back"
  };

  std::vector<CustomInstrumentComposition> m_customInstruments;

  std::inplace_vector<std::uint8_t, 4> m_playerChoices;
  std::inplace_vector<std::unique_ptr<PlayerBase>, 4> m_players;
  // Each player must send three distinct nonzero note values; releases and
  // repeated/held notes do not advance the controller check.
  std::array<std::inplace_vector<std::uint32_t, 3>, 4> m_testNotes;

  const std::string m_songName;
  std::string m_audioDirectory;

  float m_counter{};

  bool m_controllerConnected{};

  enum class Stage : std::uint8_t {
    ChoosingPlayerCount = 0,
    ChoosingInstruments,
    TestingInstruments,
  };
  Stage m_stage{Stage::ChoosingPlayerCount};
  char m_playerCount{'1'};
};

} // namespace bh
