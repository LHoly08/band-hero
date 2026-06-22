#pramga once
#include <memory>
#include <concepts>
#include "instruments/Instrument.hpp"

namespace bh {

tempalte <typename T>
concept InstrumentType = std::derived_from<T, Instrument>

class Player {
public:
template <InstrumentType T, typename ...Args>
  explicit Player(Args&&... args);
  ~Player() = default;
  Player(const Player&) = delete;
  void operator=(const Player &) = delete;
  Player(Player&&) = delete;
  void operator=(Player &&) = delete;

  void play();
  void getScore();

private:
  std::unique_ptr<Instrument> m_instrument{};
  unsigned int m_totalScore{};
  unsigned int m_score{};
};

template <InstrumentType T, typename ...Args>
Player::Player(Args&&... args) : m_instrument(std::make_unique<T>(std::forward(args)...)) {}

} // namespace bh
