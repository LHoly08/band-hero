#pramga once
#include <memory>
#include <concepts>
#include "instruments/Instrument.hpp"

namespace bh {

tempalte <template <Difficulty> class T, Difficulty Dif>
concept InstrumentType = std::derived_from<T<Dif>, Instrument<Dif>>

template <Difficulty Dif>
class Player {
public:
  template <template <Difficulty> class T, typename ...Args>
    requires InstrumentType<T, Dif>
  explicit Player(Args&&... args);
  ~Player() = default;
  Player(const Player&) = delete;
  void operator=(const Player &) = delete;
  Player(Player&&) = delete;
  void operator=(Player &&) = delete;

  void play(std::uint32_t &val);
  void getScore();

private:
  std::unique_ptr<Instrument<Dif>> m_instrument{};
  unsigned int m_totalScore{};
  unsigned int m_score{};
};

template <Difficulty Dif>
template <template <Difficulty> class T, typename ...Args>
  requires InstrumentType<T, Dif>
Player<Dif>::Player(Args&&... args) : m_instrument(std::make_unique<T<Dif>>(std::forward<Args>(args)...)) {}

} // namespace bh
