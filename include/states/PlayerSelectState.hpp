#pragma once
#include "core/StateStack.hpp"
#include "instruments/Instrument.hpp"
#include "player/Player.hpp"
#include "states/State.hpp"


namespace bh {

template <Difficulty Dif> class PlayerSelectState final : public State {
public:
  explicit PlayerSelectState(StateStack &stack, std::string songName) noexcept;
  ~PlayerSelectState() = default;

  void draw(sf::RenderTarget &target) const noexcept override;
  void handleEvents(const sf::Event &event) noexcept override;
  void update(const float dt) noexcept override;

  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::array<Player<Dif>, 4> m_players;
  std::array<std::uint8_t, 4> m_option;
  std::uint8_t m_playerCount;
};

} // namespace bh