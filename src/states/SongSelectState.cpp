#include <algorithm>
#include <filesystem>

#include "states/SongSelectState.hpp"

#include "raylib.h"

#include "core/ResourceManager.hpp"
#include "core/StateStack.hpp"

#include "states/MainMenuState.hpp"
#include "states/PlayerSelectState.hpp"

#include "ui/Button.hpp"
#include "ui/Theme.hpp"

namespace bh {

SongSelectState::SongSelectState(StateStack &stack) noexcept
    : State(stack),
      m_mainMenuButton({.x = 30, .y = 950},
                       {.x = 0, .y = 0, .width = 360, .height = 100},
                       "Main Menu"),
      m_addSongButton({.x = 30, .y = 830},
                      {.x = 0, .y = 0, .width = 360, .height = 100},
                      "Add Song") {

  std::error_code error;
  std::filesystem::directory_iterator songs(s_SongsDir, error);
  const std::filesystem::directory_iterator end;

  while (!error && songs != end) {
    if (songs->is_directory(error)) {
      m_songNames.push_back(songs->path().filename().string());
    }
    if (!error) {
      songs.increment(error);
    }
  }
  std::ranges::sort(m_songNames);

  const auto visibleCount =
      std::min(m_songNames.size(), m_songOptions.capacity());
  for (std::size_t i{}; i < visibleCount; ++i) {
    m_songOptions.emplace_back(
        Vector2{.x = 30, .y = 110.f * i + 30},
        Rectangle{.x = 0, .y = 0, .width = 360, .height = 100}, m_songNames[i]);
  }
}

void SongSelectState::refreshSongButtons() noexcept {
  if (IsMusicValid(m_song)) {
    StopMusicStream(m_song);
    UnloadMusicStream(m_song);
    m_song = {};
  }
  m_selectedOption = 0;

  for (std::size_t i{}; i < m_songOptions.size(); ++i) {
    m_songOptions[i].changeText(m_songNames[m_firstVisibleSong + i]);
    m_songOptions[i].resetInteraction();
  }
}

void SongSelectState::draw() const noexcept {

  m_mainMenuButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_addSongButton.draw<WHITE, true, 40, TextAlign::Center>();

  for (const auto &songOptionButton : m_songOptions) {
    songOptionButton.draw<WHITE, true>();
  }
}

void SongSelectState::update(float dt) noexcept {
  auto _ = dt;

  if (IsMusicValid(m_song)) {
    UpdateMusicStream(m_song);
  }
}

void SongSelectState::events() noexcept {

  const Vector2 MousePos{GetMousePosition()};
  const bool mainMenuClicked = m_mainMenuButton.updateInput(MousePos);
  const bool addSongClicked = m_addSongButton.updateInput(MousePos);

  if (mainMenuClicked) [[unlikely]] {
    m_stack.replace<MainMenuState>();

  } else if (addSongClicked) [[unlikely]] {
    // TODO: Implement the addition of songs with the path to the file
  }

  const float wheel = GetMouseWheelMove();
  const bool scrollDown = IsKeyPressed(KEY_DOWN) || wheel < 0.f;
  const bool scrollUp = IsKeyPressed(KEY_UP) || wheel > 0.f;
  const auto lastFirstSong = m_songNames.size() - m_songOptions.size();

  if (scrollDown && !scrollUp && m_firstVisibleSong < lastFirstSong) {
    ++m_firstVisibleSong;
    refreshSongButtons();
  } else if (scrollUp && !scrollDown && m_firstVisibleSong > 0) {
    --m_firstVisibleSong;
    refreshSongButtons();
  }

  for (std::uint8_t i{}; i < m_songOptions.size(); ++i) {
    auto &songOptionButton = m_songOptions[i];

    const bool clickedOption = songOptionButton.updateInput(MousePos);

    if (clickedOption) [[unlikely]] {

      if (m_selectedOption != i + 1) [[likely]] {
        m_selectedOption = i + 1;
        if (IsMusicValid(m_song)) [[likely]] {
          StopMusicStream(m_song);
          UnloadMusicStream(m_song);
        }

        m_song = LoadMusicStream(
            (s_SongsDir + songOptionButton.getText().append("/Audio/main.mp3"))
                .c_str());

        PlayMusicStream(m_song);

      } else {
        m_stack.push<PlayerSelectState>(s_SongsDir +
                                        songOptionButton.getText() + "/");

        StopMusicStream(m_song);
        UnloadMusicStream(m_song);
        m_song = {};
      }
    }
  }
}

void SongSelectState::onEnter() noexcept {
  ResourceManager::loadTextures<Textures::UI>();

  m_selectedOption = 0;
}

void SongSelectState::onExit() noexcept {
  ResourceManager::unloadTextures<Textures::UI>();

  if (IsMusicValid(m_song)) {
    StopMusicStream(m_song);
    UnloadMusicStream(m_song);
    m_song = {};
  }

  m_mainMenuButton.resetInteraction();
  m_addSongButton.resetInteraction();

  for (auto &songOptionButton : m_songOptions) {
    songOptionButton.resetInteraction();
  }
}

} // namespace bh
