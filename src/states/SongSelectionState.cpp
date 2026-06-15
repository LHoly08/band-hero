#include "states/SongSelectionState.hpp"
#include "core/StateStack.hpp"

#include <filesystem>

namespace bh {

SongSelectionState::SongSelectionState(StateStack &stack) noexcept
    : State(stack) {

  if (std::filesystem::path songsDir("songs/");
      !std::filesystem::exists(songsDir)) {
    std::filesystem::create_directory(songsDir);
    std::filesystem::current_path(songsDir);

    for (const char *difDir : {"easy/", "hard/"}) {

      if (std::filesystem::path difficultyDir(difDir);
          !std::filesystem::exists(difficultyDir)) {
        std::filesystem::create_directory(difficultyDir);
        std::filesystem::current_path(difficultyDir);

        for (const char *instDir : {"bass/", "drums/", "guitar/"}) {

          if (std::filesystem::path instrumentDir(instDir);
              !std::filesystem::exists(instrumentDir)) {
            std::filesystem::create_directory(instrumentDir);
          }
        }
      }
      std::filesystem::current_path("../");
    }
  }

  std::filesystem::path songsDir("songs/easy/drums/");
  for (const auto &entry : songsDir) {
    if (!std::filesystem::is_directory(entry)) {
      m_songs.push_back(entry.string());
    }
  }
}

void SongSelectionState::draw(sf::RenderTarget &target) const noexcept {}

void SongSelectionState::handleEvents(const sf::Event &event) noexcept {}

void SongSelectionState::update(const float dt) noexcept {}

void SongSelectionState::onEnter() noexcept {}

void SongSelectionState::onExit() noexcept {}

} // namespace bh