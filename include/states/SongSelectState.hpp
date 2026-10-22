#pragma once
#include <cstddef>
#include <inplace_vector>
#include <string>
#include <vector>

#include "core/ResourceManager.hpp"

#include "gameplay/SetList.hpp"

#include "states/State.hpp"

#include "ui/AnimatedBackground.hpp"
#include "ui/Button.hpp"

namespace bh {

class SongSelectState : public bh::State {
public:
  explicit SongSelectState(bh::StateStack &stack) noexcept;

  ~SongSelectState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<bh::ResourceManager::TextureHandle> m_textures =
      bh::ResourceManager::acquireTextures<bh::Textures::UI, bh::Textures::SongInfo>();
  bh::AnimatedBackground m_background;
  static constexpr std::string s_SongsDir{"Songs/"};

  std::inplace_vector<bh::Button, 5> m_songOptions;
  bh::Button m_mainMenuButton{{.x = 30, .y = 950},
                          {.x = 0, .y = 0, .width = 360, .height = 100},
                          "Main Menu"};
  bh::Button m_addSongButton{{.x = 30, .y = 830},
                         {.x = 0, .y = 0, .width = 360, .height = 100},
                         "Add Song"};
  bh::Button m_hardSongButton{{.x = 1530, .y = 710},
                          {.x = 0, .y = 0, .width = 360, .height = 100},
                          "Hard"};
  bh::Button m_mediumSongButton{{.x = 1530, .y = 830},
                            {.x = 0, .y = 0, .width = 360, .height = 100},
                            "Medium"};
  bh::Button m_easySongButton{{.x = 1530, .y = 950},
                          {.x = 0, .y = 0, .width = 360, .height = 100},
                          "Easy"};
  bh::Button m_startButton{{.x = 1530, .y = 30},
                       {.x = 0, .y = 0, .width = 360, .height = 100},
                       "Start"};
  bh::Button m_removeButton{{.x = 1432, .y = 32},
                        {.x = 800, .y = 300, .width = 96, .height = 96}};
  ::Music m_song{};
  bh::SetList m_setList{};
  std::inplace_vector<std::string, 4> m_selectedSongs;

  std::uint8_t m_selectedDifficulties{};
  bool m_triedInserting{false};

  static constexpr float TextScreenTime = 1.5f;
  float m_fade{};

  std::vector<std::string> m_songNames;
  std::size_t m_firstVisibleSong{};

  void refreshSongButtons() noexcept;

  static constexpr std::size_t NoSong = static_cast<std::size_t>(-1);
  // Selection belongs to the full song list, not a reusable visible button.
  std::size_t m_selectedSong{NoSong};
};

} // namespace bh
