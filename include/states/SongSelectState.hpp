#pragma once
#include <filesystem>
#include <inplace_vector>

#include "core/ResourceManager.hpp"

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class SongSelectState : public State {
public:
  inline SongSelectState(StateStack &stack) noexcept
      : State(stack),
        m_mainMenuButton({.x = 0, .y = 0},
                         {.x = 0, .y = 0, .width = 360, .height = 100},
                         "Main Menu"),
        m_addSongButton({.x = 300, .y = 300},
                        {.x = 0, .y = 0, .width = 360, .height = 100},
                        "Add Song") {}

  ~SongSelectState() override {
    ResourceManager::unloadTextures<Textures::UI>();
  };

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  static constexpr std::string s_SongsDir{"Songs/"};

  std::inplace_vector<Button, 10> m_songOptions;
  Button m_mainMenuButton;
  Button m_addSongButton;
  std::uint8_t m_selectedButton{};
};

} // namespace bh
