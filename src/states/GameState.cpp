#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <cstdint>

#include <array>
#include <bit>
#include <bitset>

#include "states/GameState.hpp"

namespace bh {

/* Gamemode Easy */

template <> void GameState<Difficulty::EASY>::update(const float dt) noexcept {

  if (std::array<unsigned char, 4> buffer{};
      m_serial.readBytes(buffer.data(), buffer.size(), 1)) {

    std::uint32_t val = std::bit_cast<std::uint32_t>(buffer);

    switch (val & 0b11) {

    case 0: // Bass
    {
      val >>= 2;

      std::bitset<NumberStrings<Difficulty::EASY>::Bass> strings{};

      for (unsigned char i{}, string = val & 0x1F;
           i < NumberStrings<Difficulty::EASY>::Bass;
           ++i, val >>= 5, string = val & 0x1F) {

        strings[i] = (string != 0);
      }

      bool _ = m_bass->getPlay(strings);
    } break;

    case 1: // Guitar
    {
      val >>= 2;

      std::bitset<NumberStrings<Difficulty::EASY>::Guitar> strings;

      for (unsigned char i{}, string = val & 0x1F;
           i < NumberStrings<Difficulty::EASY>::Guitar;
           ++i, val >>= 5, string = val & 0x1F) {

        strings[i] = (string != 0);
      }

      bool _ = m_guitar->getPlay(strings);
    } break;

    case 2: // Drums
      val >>= 2;
      break;

    case 3: // Custom
      val >>= 2;
      break;
    }
  }
}

template <>
void GameState<Difficulty::EASY>::draw(
    sf::RenderTarget &target) const noexcept {

  target.clear(sf::Color::Red);
}

/* Gamemode Hard */

template <> void GameState<Difficulty::HARD>::update(const float dt) noexcept {

  if (std::array<unsigned char, 4> buffer{};
      m_serial.readBytes(buffer.data(), buffer.size(), 1)) {

    std::uint32_t val = std::bit_cast<std::uint32_t>(buffer);

    switch (val & 0b11) {

    case 0: // Bass
    {
      val >>= 2;

      bool _ = m_bass->getPlay(val);
    } break;

    case 1: // Guitar
    {
      val >>= 2;

      bool _ = m_guitar->getPlay(val);
    } break;

    case 2: // Drums
    {
      val >>= 2;

      bool _ = m_drums->getPlay(val);
    } break;

    case 3: // Custom
    {
      val >>= 2;

    } break;
    }
  }
}

template <>
void GameState<Difficulty::HARD>::draw(
    sf::RenderTarget &target) const noexcept {

  target.clear(sf::Color::Green);
}

} // namespace bh
