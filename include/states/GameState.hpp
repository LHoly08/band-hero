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
#include "gameplay/SetList.hpp"

#include "serial/serialib.h"

#include "states/MainMenuState.hpp"
#include "states/PauseMenuState.hpp"
#include "states/State.hpp"

namespace bh {

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
class GameEndState;

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
class GameState final : public State {
public:
  inline GameState(
      StateStack &stack,
      std::array<std::unique_ptr<PlayerBase>, PlayerCount> &&players,
      std::string &&filename, SetList setList,
      std::uint32_t completedSongs = 0, bool allSongsCompleted = true) noexcept;
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
  SetList m_setList{};
  std::uint32_t m_completedSongs{};
  bool m_allSongsCompleted{true};

  const float m_duration{};
  float m_time{};
  bool m_audioStarted{false};
};

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
GameState<PlayerCount>::GameState(
    StateStack &stack,
    std::array<std::unique_ptr<PlayerBase>, PlayerCount> &&players,
    std::string &&filename, SetList setList, std::uint32_t completedSongs,
    bool allSongsCompleted) noexcept
    : State(stack), m_players(std::move(players)),
      m_setList(std::move(setList)), m_completedSongs(completedSongs),
      m_allSongsCompleted(allSongsCompleted), m_duration([filename] -> float {
        Music temp = LoadMusicStream((filename + "main.mp3").c_str());
        const float duration = GetMusicTimeLength(temp);
        UnloadMusicStream(temp);
        return duration + 1;
      }()) {
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
      "bass.wav",   "drums.wav", "guitar.wav",
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
  if (m_time >= m_duration) {
    return;
  }

  for (const auto &player : m_players) {
    player->draw();
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameState<PlayerCount>::update(float dt) noexcept {
  m_time += dt;

  if (m_time >= m_duration) {
    // Include the final frame so notes leaving their hit window are counted.
    for (auto &player : m_players) {
      player->update(dt);
    }
    m_stack.push<GameEndState<PlayerCount>>(std::move(m_players),
                                          std::move(m_setList),
                                          m_completedSongs + 1,
                                          m_allSongsCompleted);
    return;
  }

  for (auto &audio : m_audios) {
    UpdateMusicStream(audio);
  }

  for (auto &player : m_players) {
    player->update(dt);
  }


  std::uint32_t packet{};
  const int received = m_serial.readBytes(&packet, sizeof(packet), 1);

  if (received == sizeof(packet)) [[likely]] {
    if constexpr (std::endian::native == std::endian::big) {
      packet = std::byteswap(packet);
    }
    const auto index = packet & 0b11;
    if (packet < PlayerCount) {
      m_players[index]->play(packet >> 2);
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
  if (!Settings::getSerialPort().empty() &&
      m_serial.openDevice(Settings::getSerialPort().c_str(),
                          Settings::getSerialBaudRate()) != 1) {
    m_serial.closeDevice();
  }
  if (m_serial.isDeviceOpen()) {
    m_serial.flushReceiver();
  }

  for (auto &player : m_players) {
    player->resetControllerState();
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
    // The end screen takes ownership before the queued transition exits us.
    if (player) {
      player->pauseInstrument(true);
      player->controlAudio(false);
    }
  }
  for (auto &audio : m_audios) {
    PauseMusicStream(audio);
  }
}

} // namespace bh

// Define the end screen after GameState so each transition sees both templates.
#include "states/GameEndState.hpp"
