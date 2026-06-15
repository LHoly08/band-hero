#pragma once
#include "core/StateStack.hpp"
#include "states/State.hpp"

namespace bh {

class SongSelectionState final : public State {
public:
  explicit SongSelectionState(StateStack &stack) noexcept;
  ~SongSelectionState() = default;

  void draw(sf::RenderTarget &target) const noexcept override;
  void handleEvents(const sf::Event &event) noexcept override;
  void update(const float dt) noexcept override;

  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<std::string> m_songs;
};
} // namespace bh