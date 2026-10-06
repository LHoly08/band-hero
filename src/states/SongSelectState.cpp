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

#include <cstdio>

namespace bh {

SongSelectState::SongSelectState(StateStack &stack) noexcept
    : State(stack) {

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
  // Scrolling changes only the viewport. Keep the selected song and its stream
  // alive; release button presses so a recycled row cannot select another song.
  for (std::size_t i{}; i < m_songOptions.size(); ++i) {
    m_songOptions[i].changeText(m_songNames[m_firstVisibleSong + i]);
    m_songOptions[i].resetInteraction();
  }
}

void SongSelectState::draw() const noexcept {
  m_background.draw();

  m_mainMenuButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_addSongButton.draw<WHITE, true, 40, TextAlign::Center>();

  for (std::size_t i = 0; i < m_songOptions.size(); ++i) {
    if (m_firstVisibleSong + i == m_selectedSong) {
      m_songOptions[i].draw<theme::Highlight, true>();
    } else {
      m_songOptions[i].draw<WHITE, true>();
    }
  }

  //ResourceManager::drawText();

  if (m_selectedSong != NoSong) {
    m_hardSongButton.draw<WHITE, true, 40, TextAlign::Center>();
    m_mediumSongButton.draw<WHITE, true, 40, TextAlign::Center>();
    m_easySongButton.draw<WHITE, true, 40, TextAlign::Center>();
  }
}

void SongSelectState::update(float dt) noexcept {
  m_background.update(dt);

  if (IsMusicValid(m_song)) {
    UpdateMusicStream(m_song);
  }
}

void SongSelectState::events() noexcept {

  const Vector2 MousePos{GetMousePosition()};
  const bool enabled = m_selectedSong != NoSong;

  const bool mainMenuClicked = m_mainMenuButton.updateInput(MousePos);
  const bool addSongClicked = m_addSongButton.updateInput(MousePos);
  const bool hardClicked = m_hardSongButton.updateInput(MousePos, enabled);
  const bool mediumClicked = m_mediumSongButton.updateInput(MousePos, enabled);
  const bool easyClicked = m_easySongButton.updateInput(MousePos, enabled);


  if (mainMenuClicked) [[unlikely]] {
    m_stack.replace<MainMenuState>();

  } else if (addSongClicked) [[unlikely]] {
    // TODO: Implement the addition of songs with the path to the file
  }
  if (enabled) {

    std::string path = s_SongsDir;
    path.append(m_songNames[m_selectedSong]);

    constexpr auto getButtonDifficulty = [](std::meta::info metaIdentifier) consteval -> auto {
      const std::string_view identifier = std::meta::identifier_of(metaIdentifier);
      std::string difficulty{'/'};

      for (auto it = identifier.begin(); it != identifier.end() && !(*it >= 'A' && *it <= 'Z'); ++it) {
        difficulty += *it;
      }
      difficulty.at(1) = difficulty.at(1) - 32;
      difficulty += '/';

      return std::define_static_string(difficulty);
    };

    if (hardClicked) [[unlikely]] {
      path.append(getButtonDifficulty(^^hardClicked));
      std::puts(path.c_str());
      m_stack.push<PlayerSelectState>(std::move(path));

      StopMusicStream(m_song);
      UnloadMusicStream(m_song);
      m_song = {};

    } else if (mediumClicked) [[unlikely]] {
      path.append(getButtonDifficulty(^^mediumClicked));
      std::puts(path.c_str());
      m_stack.push<PlayerSelectState>(std::move(path));

      StopMusicStream(m_song);
      UnloadMusicStream(m_song);
      m_song = {};

    } else if (easyClicked) [[unlikely]] {
      path.append(getButtonDifficulty(^^easyClicked));
      std::puts(path.c_str());
      m_stack.push<PlayerSelectState>(std::move(path));

      StopMusicStream(m_song);
      UnloadMusicStream(m_song);
      m_song = {};

    }
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

      const auto songIndex = m_firstVisibleSong + i;
      if (m_selectedSong != songIndex) [[likely]] {
        m_selectedSong = songIndex;

        if (IsMusicValid(m_song)) [[likely]] {
          StopMusicStream(m_song);
          UnloadMusicStream(m_song);
        }

        m_song = LoadMusicStream(
            (s_SongsDir + m_songNames[m_selectedSong] + "/Audio/main.mp3")
                .c_str());

        PlayMusicStream(m_song);

      }
    }
  }
}

void SongSelectState::onEnter() noexcept {
  ResourceManager::loadTextures<Textures::UI>();

  m_selectedSong = NoSong;
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
