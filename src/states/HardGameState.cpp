#include <SFML/Window/Keyboard.hpp>
#include <cstdint>

#include <array>
#include <bit>

#include "states/HardGameState.hpp"

#include "core/StateStack.hpp"

#include "settings/Device.hpp"

#include "states/PauseMenuState.hpp"

namespace bh {

HardGameState::HardGameState(StateStack &stack) noexcept : State(stack) {
  m_serial.openDevice(Device::PortName().c_str(), Device::BaudRate());
}

void HardGameState::draw(sf::RenderTarget &target) const noexcept {}

void HardGameState::handleEvents(const sf::Event &event) noexcept {
  if (const auto *key = event.getIf<sf::Event::KeyPressed>()) {
    switch (key->scancode) {
    case sf::Keyboard::Scancode::Escape:
      m_stack.push<PauseMenuState>();
      break;

    default:
      break;
    }
  }
}

void HardGameState::update(float dt) noexcept {

  std::array<unsigned char, 4> buffer{};
  m_serial.readBytes(buffer.data(), buffer.size(), 1);

  std::uint32_t val = std::bit_cast<std::uint32_t>(buffer);

  switch (val & 0b11) {

  case 0: // Bass
    val >>= 2;
    for (unsigned char i{}, string = val & 0x1F; i < 6;
         ++i, val >>= 5, string = val & 0x1F) {
      m_bass->getPlay(i, string);
    }
    break;

  case 1: // Guitar
    val >>= 2;
    break;

  case 2: // Drums
    val >>= 2;
    break;

  case 3: // Custom
    val >>= 2;
    break;
  }
}

void HardGameState::onEnter() noexcept {}

void HardGameState::onExit() noexcept {}

} // namespace bh
