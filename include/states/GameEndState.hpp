#pragma once

#include <array>
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

namespace bh {

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
class GameEndState final : public State {
public:
  inline GameEndState(
      StateStack &stack,
      std::array<std::unique_ptr<PlayerBase>, PlayerCount> players,
      SetList setList) noexcept
      : State(stack), m_players(std::move(players)),
        m_setList(std::move(setList)), m_nextSongName([this] -> std::string {
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
      ResourceManager::acquireTextures<Textures::UI, Textures::Songs,
                                       Textures::Achievements>();
  AnimatedBackground m_background;
  std::array<std::unique_ptr<PlayerBase>, PlayerCount> m_players;
  SetList m_setList;

  Button m_playButton{{.x = 30, .y = 430}, {0, 700, 460, 100}, "Next Song"};

  Button m_quitButton{{.x = 30, .y = 670},
                      {.x = 0, .y = 700, .width = 460, .height = 100},
                      "Quit"};
  Button m_mainMenuButton{{.x = 30, .y = 550},
                          {.x = 0, .y = 700, .width = 460, .height = 100},
                          "Main Menu"};
  std::string m_nextSongName;
  std::uint8_t m_nextDifficulty;

  float m_delay{};
  bool m_start{false};
};

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameEndState<PlayerCount>::draw() const noexcept {
  m_background.draw();

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

    ResourceManager::drawImage<Textures::Songs::Difficulty>(
        {((m_nextDifficulty - 1) * 100.f), 0, 100, 100},
        {row.x + 360.f, row.y});
  }
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameEndState<PlayerCount>::update(float dt) noexcept {
  m_delay += dt;

  m_background.update(dt);

  if (m_setList.empty()) {
    if (m_delay >= 3.0f) {
      m_stack.reset<MainMenuState>();
    }
    return;
  }

  if (m_start && m_delay >= 3.0f) {
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
                                          std::move(m_setList));
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
}

template <std::uint8_t PlayerCount>
  requires MaxPlayerAmount<PlayerCount>
void GameEndState<PlayerCount>::onExit() noexcept {
  m_mainMenuButton.resetInteraction();
  m_quitButton.resetInteraction();
  m_playButton.resetInteraction();
}

} // namespace bh
