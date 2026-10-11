#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <iterator>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

#include "raylib.h"

#include "core/StateStack.hpp"

#include "gameplay/Player.hpp"
#include "gameplay/SetList.hpp"

#include "states/GameState.hpp"
#include "states/MainMenuState.hpp"
#include "states/State.hpp"

#include "ui/AnimatedBackground.hpp"
#include "ui/Button.hpp"
#include "ui/Theme.hpp"

namespace bh {

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
class GameEndState final : public State {
public:
  inline GameEndState(
      StateStack &stack,
      std::array<std::unique_ptr<PlayerBase>, PlayerCount> players,
      SetList setList, std::uint32_t completedSongs = 1,
      bool allSongsCompleted = true) noexcept
      : State(stack), m_players(std::move(players)),
        m_setList(std::move(setList)), m_completedSongs(completedSongs),
        m_allSongsCompleted(allSongsCompleted), m_nextSongName([this] -> std::string {
          if (m_setList.empty()) {
            return "";
          }
          std::string_view songDirectory = m_setList.getData() + 6;
          std::string_view songName =
              songDirectory.substr(0, songDirectory.find('/'));

          return std::string{songName};
        }()),
        m_nextDifficulty([this] -> std::uint8_t {
          if (m_setList.empty()) {
            return 0;
          }
          std::string_view songPath =
              m_setList.getData() + 7 + m_nextSongName.size();
          std::string_view difficulty{
              songPath.data(),
              static_cast<std::size_t>(
                  std::distance(songPath.begin(), songPath.end()) - 1)};
          static constexpr std::array<std::string_view, 3> Difficulties{
              "Easy", "Medium", "Hard"};

          for (std::uint8_t i{}; i < Difficulties.size(); ++i) {
            if (difficulty == Difficulties[i]) {
              return i + 1;
            }
          }
          std::unreachable();
        }()) {

    if (m_setList.empty()) {
      m_quitButton.setPosition({.x = 30, .y = 550});
      m_mainMenuButton.setPosition({.x = 30, .y = 430});
    }
  }
  ~GameEndState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<ResourceManager::TextureHandle> m_textures =
      ResourceManager::acquireTextures<Textures::UI, Textures::SongInfo,
                                       Textures::Achievements>();
  AnimatedBackground m_background;
  std::array<std::unique_ptr<PlayerBase>, PlayerCount> m_players;
  SetList m_setList;
  std::uint32_t m_completedSongs{};
  bool m_allSongsCompleted{true};
  bool m_resultsReady{};
  std::vector<Achievements> m_earnedMedals;
  std::vector<Achievements> m_newMedals;

  Button m_playButton{{.x = 30, .y = 430}, {0, 700, 460, 100}, "Next Song"};

  Button m_quitButton{{.x = 30, .y = 670},
                      {.x = 0, .y = 700, .width = 460, .height = 100},
                      "Quit"};
  Button m_mainMenuButton{{.x = 30, .y = 550},
                          {.x = 0, .y = 700, .width = 460, .height = 100},
                          "Main Menu"};
  std::string m_nextSongName;
  std::uint8_t m_nextDifficulty;

  static constexpr float NextSongDelay = 3.f;
  float m_delay{};
  bool m_start{false};
};

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameEndState<PlayerCount>::draw() const noexcept {
  m_background.draw();

  ResourceManager::drawText<Fonts_t::Buttons>(
      "SONG COMPLETE", {30, 150}, 42, theme::Text);
  ResourceManager::drawText<Fonts_t::Buttons>(
      "Song " + std::to_string(m_completedSongs), {30, 210}, 26,
      theme::MutedText);
  ResourceManager::drawSettingsSkin<Textures::UI::Panel>(
      {0, 0, 768, 384}, {570, 120, 1320, 870});
  ResourceManager::drawText<Fonts_t::Buttons>(
      "ACHIEVEMENTS EARNED", {610, 155}, 36, theme::Text);
  const std::string newCount = std::to_string(m_newMedals.size()) + " NEW";
  const float newWidth =
      ResourceManager::measureText<Fonts_t::Buttons>(newCount, 24);
  ResourceManager::drawText<Fonts_t::Buttons>(
      newCount, {1850 - newWidth, 162}, 24, theme::Highlight);

  for (std::size_t i = 0; i < m_players.size(); ++i) {
    const float x = 610 + i * (1240.f / PlayerCount);
    const auto &player = m_players[i];
    ResourceManager::drawText<Fonts_t::Buttons>(
        "PLAYER " + std::to_string(i + 1), {x, 225}, 22, theme::Primary);
    const auto percent = static_cast<int>(std::round(player->getCompletion() * 100));
    const std::string result = std::to_string(player->getHitNoteCount()) + " / " +
                              std::to_string(player->getLoadedNoteCount()) +
                              " hits  (" + std::to_string(percent) + "%)";
    const float width = ResourceManager::measureText<Fonts_t::Buttons>(result, 24);
    const float size = std::min(24.f, (1240.f / PlayerCount - 20) * 24.f /
                                        std::max(1.f, width));
    ResourceManager::drawText<Fonts_t::Buttons>(
        result, {x, 260}, size, theme::Text);
  }

  std::string_view hint = "Hover over a medal to see its unlock requirement.";
  if (m_earnedMedals.empty()) {
    ResourceManager::drawText<Fonts_t::Buttons>(
        "No achievements earned", {610, 365}, 32, theme::MutedText);
    hint = "Complete a nonempty chart to earn song achievements.";
  }
  for (std::size_t i = 0; i < m_earnedMedals.size(); ++i) {
    const auto medal = m_earnedMedals[i];
    const float x = 610 + (i % 5) * 248.f;
    const float y = 335 + (i / 5) * 148.f;
    AchievementsManager::drawMedal(medal, Rectangle{x + 70, y, 100, 100});
    const auto name = AchievementsManager::name(medal);
    const float width = ResourceManager::measureText<Fonts_t::Buttons>(name, 24);
    const float size = std::min(24.f, 228.f * 24.f / std::max(1.f, width));
    const float fitted = ResourceManager::measureText<Fonts_t::Buttons>(name, size);
    ResourceManager::drawText<Fonts_t::Buttons>(
        name, {x + (240 - fitted) / 2, y + 105}, size, theme::Text);
    if (std::ranges::find(m_newMedals, medal) != m_newMedals.end()) {
      ResourceManager::drawText<Fonts_t::Buttons>(
          "NEW", {x + 175, y + 12}, 18, theme::Highlight);
    }
    const auto position = scaledSize(Vector2{x, y});
    const auto extent = scaledSize(Vector2{240, 135});
    if (CheckCollisionPointRec(GetMousePosition(),
                               {position.x, position.y, extent.x, extent.y})) {
      hint = AchievementsManager::description(medal);
    }
  }
  ResourceManager::drawText<Fonts_t::Buttons>(
      hint, {610, 950}, 22, theme::MutedText);

  m_mainMenuButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_quitButton.draw<WHITE, true, 40, TextAlign::Center>();

  if (!m_setList.empty()) {
    m_playButton.draw<WHITE, true, 40, TextAlign::Center>();

    const Rectangle row{30, 310, 360, 100};
    ResourceManager::drawSettingsSkin<Textures::UI::Panel>({0, 0, 768, 384},
                                                           row);
    // Match the song buttons' padding, vertical alignment and text fitting.
    constexpr float padding = 20.f;
    float fontSize = 40.f;
    const float textWidth = ResourceManager::measureText<Fonts_t::Buttons>(
        m_nextSongName, fontSize);
    const float availableWidth = row.width - 2.f * padding;
    if (textWidth > availableWidth) {
      fontSize *= availableWidth / textWidth;
    }
    ResourceManager::drawText<Fonts_t::Buttons>(
        m_nextSongName,
        {row.x + padding, row.y + (row.height - fontSize) / 2.f}, fontSize,
        theme::Text);

    ResourceManager::drawImage<Textures::SongInfo::Difficulty>(
        {((m_nextDifficulty - 1) * 100.f), 0, 100, 100},
        {row.x + 360.f, row.y});

    if (m_start) {
      ResourceManager::drawSettingsSkin<Textures::UI::Panel>(
          {0, 0, 768, 384}, {570, 410, 780, 260});
      const auto centeredText = [](std::string_view text, float y, float size,
                                   Color color) {
        const float width =
            ResourceManager::measureText<Fonts_t::Buttons>(text, size);
        ResourceManager::drawText<Fonts_t::Buttons>(
            text, {960 - width / 2, y}, size, color);
      };
      const float remaining = std::max(0.f, NextSongDelay - m_delay);
      const int seconds =
          std::clamp(static_cast<int>(std::ceil(remaining)), 1, 3);
      centeredText("NEXT SONG", 435, 26, theme::Text);
      centeredText(std::to_string(seconds), 475, 100, theme::Highlight);
      const auto position = scaledSize(Vector2{710, 610});
      const auto size = scaledSize(Vector2{500, 10});
      DrawRectangleRounded({position.x, position.y, size.x, size.y}, 1.f, 12,
                           theme::Border);
      if (remaining > 0.f) {
        DrawRectangleRounded({position.x, position.y,
                              size.x * remaining / NextSongDelay, size.y},
                             1.f, 12, theme::Highlight);
      }
    }
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameEndState<PlayerCount>::update(float dt) noexcept {
  m_delay += dt;

  m_background.update(dt);

  if (m_setList.empty()) {
    if (m_delay >= 30.0f) {
      m_stack.reset<MainMenuState>();
    }
    return;
  }

  if (m_start && m_delay >= NextSongDelay) {
    std::array<std::unique_ptr<PlayerBase>, PlayerCount> newPlayers;
    std::string nextSong = m_setList.getCharts();

    for (std::tuple<std::unique_ptr<PlayerBase> &,
                    std::unique_ptr<PlayerBase> &>
             player : std::ranges::views::zip(m_players, newPlayers)) {
      std::get<1>(player) = std::get<0>(player)->changeSong(nextSong);
    }

    std::string nextAudioDirectory = m_setList.getAudio();
    m_setList.pop();
    m_stack.reset<GameState<PlayerCount>>(std::move(newPlayers),
                                          std::move(nextAudioDirectory),
                                          std::move(m_setList), m_completedSongs,
                                          m_allSongsCompleted);
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameEndState<PlayerCount>::events() noexcept {
  const Vector2 MousePos = GetMousePosition();

  const bool mainMenuClicked = m_mainMenuButton.updateInput(MousePos);
  const bool quitClicked = m_quitButton.updateInput(MousePos);
  const bool playClicked =
      m_playButton.updateInput(MousePos, !m_setList.empty() && !m_start);

  if (mainMenuClicked) [[unlikely]] {
    m_stack.reset<MainMenuState>();
  } else if (quitClicked) [[unlikely]] {
    m_stack.quit = true;
  } else if (playClicked) [[unlikely]] {
    m_start = true;
    m_delay = 0;
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameEndState<PlayerCount>::onEnter() noexcept {
  m_delay = 0;
  if (m_resultsReady) {
    return;
  }
  bool bandCompleted = true;
  for (auto &player : m_players) {
    player->finishSong();
    bandCompleted = bandCompleted && player->hasCompletedSong();
    const auto earned = player->getUnlockedAchievements();
    m_earnedMedals.insert(m_earnedMedals.end(), earned.begin(), earned.end());
  }
  m_allSongsCompleted = m_allSongsCompleted && bandCompleted;
  if (bandCompleted) {
    if constexpr (PlayerCount >= 2) {
      m_earnedMedals.push_back(Achievements::BandTogether);
    }
    if constexpr (PlayerCount == 4) {
      m_earnedMedals.push_back(Achievements::FourPlayer);
    }
    if (m_setList.empty() && m_completedSongs >= 2 && m_allSongsCompleted) {
      m_earnedMedals.push_back(Achievements::Setlist);
    }
  }
  std::ranges::sort(m_earnedMedals);
  const auto duplicates = std::ranges::unique(m_earnedMedals);
  m_earnedMedals.erase(duplicates.begin(), duplicates.end());
  m_newMedals = AchievementsManager::get().unlock(m_earnedMedals);
  m_resultsReady = true;
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameEndState<PlayerCount>::onExit() noexcept {
  m_mainMenuButton.resetInteraction();
  m_quitButton.resetInteraction();
  m_playButton.resetInteraction();
}

} // namespace bh
