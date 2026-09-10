#pragma once
#include <cstdint>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "raylib.h"

namespace bh {

class Settings {
public:
  inline static Settings &get() noexcept {
    static Settings s_instance{};
    return s_instance;
  }

  inline static Color getNoteTint(std::uint8_t index) noexcept {
    return get().iGetNoteTint(index);
  }
  inline static const std::string &getSerialPort() noexcept {
    return get().serialPort;
  }
  inline static std::uint32_t getSerialBaudRate() noexcept {
    return get().serialBaudRate;
  }
  static std::vector<std::string> getAvailableSerialPorts();
  static constexpr std::array<std::uint32_t, 11> supportedBaudRates{
      110, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

  inline static void setSerialPort(std::string port) {
    get().serialPort = std::move(port);
    get().iSaveSettings();
  }
  inline static bool setSerialBaudRate(std::uint32_t baudRate) noexcept {
    if (!get().iSetSerialBaudRate(baudRate)) {
      return false;
    }
    get().iSaveSettings();
    return true;
  }

  inline static void loadSettings() noexcept { return get().iLoadSettings(); }
  inline static void saveSettings() noexcept { return get().iSaveSettings(); }
  inline static void defaultSettings() noexcept {
    return get().iDefaultSettings();
  }
  inline static bool detectSerialPort() noexcept {
    const bool found = get().iDetectSerialPort();
    get().iSaveSettings();
    return found;
  }

  inline static void defaultStartupSettings() noexcept {
    return get().iDefaultStartupSettings();
  }

  ~Settings() = default;

  Settings operator=(const Settings &) = delete;
  Settings(const Settings &) = delete;
  Settings operator=(Settings &&) = delete;
  Settings(Settings &&) = delete;

  static constexpr std::string startupFile{"startup.bin"};

private:
  Settings();

  void iDefaultSettings() noexcept;
  void iLoadSettings() noexcept;
  void iSaveSettings() noexcept;
  bool iDetectSerialPort() noexcept;
  bool iSetSerialBaudRate(std::uint32_t baudRate) noexcept;

  void iDefaultStartupSettings() noexcept;

  inline Color iGetNoteTint(std::uint8_t index) const noexcept {
    try {
      return guitarBassColors.at(index);
    } catch (...) {
      return guitarBassColors[0];
    }
  }

  static constexpr std::string fileName{"settings.bin"};

  std::array<Color, 6> guitarBassColors{RED,   ORANGE, YELLOW,
                                        GREEN, BLUE,   PURPLE};
  std::string serialPort;
  std::uint32_t serialBaudRate{115200};
};

} // namespace bh
