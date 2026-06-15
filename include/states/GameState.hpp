#pragma once
#include <SFML/Window/Event.hpp>

#include <memory>
#include <optional>

#include "core/StateStack.hpp"

#include "instruments/Bass.hpp"
#include "instruments/Drums.hpp"
#include "instruments/Guitar.hpp"
#include "instruments/Instrument.hpp"

#include "serial/serialib.h"

#include "settings/Device.hpp"

#include "states/ConnectionState.hpp"
#include "states/PauseMenuState.hpp"
#include "states/State.hpp"

namespace bh {

template <Difficulty Dif> class GameState final : public State {
public:
  explicit GameState(StateStack &stack, std::string songName,
                     std::unique_ptr<Instrument<Dif>> instrumet);
  ~GameState() = default;

  void draw(sf::RenderTarget &target) const noexcept override;
  void handleEvents(const sf::Event &event) noexcept override;
  void update(const float dt) noexcept override;

  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::unique_ptr<Instrument<Dif>> m_custom;

  serialib m_serial;

  Bass<Dif> m_bass;
  Guitar<Dif> m_guitar;
  Drums<Dif> m_drums;
};

template <Difficulty Dif>
GameState<Dif>::GameState(StateStack &stack, std::string songName,
                          std::unique_ptr<Instrument<Dif>> instrumet)
    : State(stack), m_custom(std::move(instrumet)), m_bass(songName),
      m_guitar(songName), m_drums(songName) {}

template <Difficulty Dif>
void GameState<Dif>::handleEvents(const sf::Event &event) noexcept {

  if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {

    switch (keyPressed->scancode) {
    case sf::Keyboard::Scancode::Escape:
      m_stack.push<PauseMenuState>();
      break;

    default:
      break;
    }
  }
}

template <Difficulty Dif> void GameState<Dif>::update(float dt) noexcept {

  if (std::array<unsigned char, 4> buffer{};
      m_serial.readBytes(buffer.data(), buffer.size(), 1)) {

    std::uint32_t val = std::bit_cast<std::uint32_t>(buffer);

    switch (val & 0b11) {

    case 0: // Bass
    {
      val >>= 2;

      if constexpr (Dif == Difficulty::EASY) {

        for (std::uint8_t i{}, string = (val & 0x1F);
             i < m_bass.getNumberStrings();
             string = (val >> (++i * 5)) & 0x1F) {

          val |= (string != 0) * (0x1F << (i * 5));
        }
      }
      bool _ = m_bass.getPlay(val);
    } break;

    case 1: // Guitar
    {
      val >>= 2;

      if constexpr (Dif == Difficulty::EASY) {

        for (std::uint8_t i{}, string = (val & 0x1F);
             i < m_guitar.getNumberStrings();
             string = (val >> (++i * 5)) & 0x1F) {

          val |= (string != 0) * (0x1F << (i * 5));
        }
      }

      bool _ = m_guitar.getPlay(val);
    } break;

    case 2: // Drums
    {
      val >>= 2;

      bool _ = m_drums.getPlay(val);
    } break;

    case 3: // Custom
    {
      val >>= 2;

      if (m_custom) {
        bool _ = m_custom->getPlay(val);
      }

    } break;
    }
  }
}

template <Difficulty Dif> void GameState<Dif>::onEnter() noexcept {

  auto serialPort = Device::PortName();

  if (serialPort) {
    m_serial.openDevice(Device::PortName()->c_str(), Device::BaudRate());
  } else {
    m_stack.push<ConnectionState>();
  }
}

template <Difficulty Dif> void GameState<Dif>::onExit() noexcept {}

} // namespace bh
