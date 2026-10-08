#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <inplace_vector>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "raylib.h"

#include "config/Settings.hpp"

#include "core/ResourceManager.hpp"

#include "gameplay/Player.hpp"

#include "serial/serialib.h"

#include "states/PauseMenuState.hpp"
#include "states/State.hpp"

namespace bh {

template <std::uint8_t T>
concept MaxPlayerAmount = (T != 0) && (T <= 4);

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
class GameState final : public State {
public:
  inline GameState(
      StateStack &stack,
      std::array<std::unique_ptr<PlayerBase>, PlayerCount> &&players,
      std::string &&filename) noexcept;
  ~GameState() override;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  ResourceManager::TextureHandle m_notes =
      ResourceManager::acquireTexture<Textures::Gameplay::Notes>();
  serialib m_serial;
  std::array<std::unique_ptr<PlayerBase>, PlayerCount> m_players;
  std::inplace_vector<Music, 6> m_audios;
  bool m_audioStarted{false};
  float m_time{};
  const float m_duration{};
};

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
GameState<PlayerCount>::GameState(
    StateStack &stack,
    std::array<std::unique_ptr<PlayerBase>, PlayerCount> &&players,
    std::string &&filename) noexcept
    : State(stack), m_players(std::move(players)) {
  // Only the standard instruments own one of the six backing stems.
  // Custom instruments load their own custom_N.wav files.
  std::array<bool, 3> playerInstruments{};
  if (!filename.empty() && filename.back() != '/' && filename.back() != '\\') {
    filename.push_back('/');
  }

  for (auto &player : m_players) {
    player->loadAudio(filename);
    const auto type = player->getInstrumentType();
    if (type < playerInstruments.size()) {
      playerInstruments[type] = true;
    }
  }

  static constexpr std::array<std::string_view, 6> AudioNames{
      "bass.wav", "drums.wav", "guitar.wav",
      "vocals.wav", "piano.wav", "other.wav"};

  for (std::uint8_t i{}; i < AudioNames.size(); ++i) {
    if (i < playerInstruments.size() && playerInstruments[i]) {
      continue;
    }
    std::string audioPath = filename;
    audioPath.append(AudioNames[i]);
    auto audio = LoadMusicStream(audioPath.c_str());
    if (IsMusicValid(audio)) {
      audio.looping = false;
      m_audios.emplace_back(audio);
    }
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
GameState<PlayerCount>::~GameState() {
  for (auto &audio : m_audios) {
    StopMusicStream(audio);
    UnloadMusicStream(audio);
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::draw() const noexcept {
  for (const auto &player : m_players) {
    player->draw();
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::update(float dt) noexcept {
  for (auto &audio : m_audios) {
    UpdateMusicStream(audio);
  }

  for (auto &player : m_players) {
    player->update(dt);
  }

  // Controller packets are four-byte little-endian words: the low two bits
  // identify player 0-3 and the remaining 30 bits encode that player's input.
  // Accept only complete four-byte reads. Short reads consume their bytes but
  // are discarded rather than retained for the next update.
  if (std::uint32_t buffer{};
      m_serial.readBytes(&buffer, sizeof(buffer), 1) == sizeof(buffer)) {

    if constexpr (std::endian::native == std::endian::big) {
      buffer = std::byteswap(buffer);
    }

    if (std::uint8_t index = buffer & 0b11; index < PlayerCount) [[likely]] {
      buffer >>= 2;

      m_players[index]->play(buffer);
    }
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::events() noexcept {

  if (IsKeyPressed(KEY_ESCAPE)) [[unlikely]] {
    m_stack.push<PauseMenuState>();
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::onEnter() noexcept {
  m_serial.openDevice(Settings::getSerialPort().c_str(),
                      Settings::getSerialBaudRate());

  for (auto &player : m_players) {
    player->pauseInstrument(false);
  }
  for (auto &player : m_players) {
    if (m_audioStarted) {
      player->controlAudio(true);
    } else {
      player->startAudio();
    }
  }
  for (auto &audio : m_audios) {
    if (m_audioStarted) {
      ResumeMusicStream(audio);
    } else {
      PlayMusicStream(audio);
    }
  }
  m_audioStarted = true;
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::onExit() noexcept {
  m_serial.closeDevice();

  for (auto &player : m_players) {
    player->pauseInstrument(true);
  }
  for (auto &player : m_players) {
    player->controlAudio(false);
  }
  for (auto &audio : m_audios) {
    PauseMusicStream(audio);
  }
}

} // namespace bh
