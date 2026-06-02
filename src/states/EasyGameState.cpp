#include <cstdint>

#include <array>
#include <bit>

#include "core/StateStack.hpp"

#include "settings/Device.hpp"

#include "states/EasyGameState.hpp"

namespace bh {

EasyGameState::EasyGameState(StateStack &stack) noexcept : State(stack) {
  m_serial.openDevice(Device::PortName().c_str(), Device::BaudRate());
}

void EasyGameState::draw(sf::RenderTarget &target) const noexcept {}

void EasyGameState::handleEvents(const sf::Event &event) noexcept {}

void EasyGameState::update(float dt) noexcept {

  std::array<unsigned char, 4> buffer{};
  m_serial.readBytes(buffer.data(), buffer.size(), 1);

  std::uint32_t val = std::bit_cast<std::uint32_t>(buffer);

  switch (val & 0b11) {

  case 0: // Bass
    val >>= 2;
    for (unsigned char i{}, string = val & 0x1F; i < 6;
         ++i, val >>= 5, string = val & 0x1F) {
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

void EasyGameState::onEnter() noexcept {}

void EasyGameState::onExit() noexcept {}

} // namespace bh
