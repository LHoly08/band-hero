#pragma once
#include <cstddef>
#include <inplace_vector>
#include <string>
#include <vector>

#include "core/ResourceManager.hpp"

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class SongSelectState : public State {
public:
  explicit SongSelectState(StateStack &stack) noexcept;

  ~SongSelectState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  static constexpr std::string s_SongsDir{"Songs/"};

  std::inplace_vector<Button, 5> m_songOptions;
  Button m_mainMenuButton;
  Button m_addSongButton;
  Button m_hardSongButton;
  Button m_mediumSongButton;
  Button m_easySongButton;
  Music m_song{};

  std::vector<std::string> m_songNames;
  std::size_t m_firstVisibleSong{};

  void refreshSongButtons() noexcept;

  static constexpr std::size_t NoSong = static_cast<std::size_t>(-1);
  // Selection belongs to the full song list, not a reusable visible button.
  std::size_t m_selectedSong{NoSong};
};

} // namespace bh
