#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <meta>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "raylib.h"

#include "gameplay/instruments/Instrument.hpp"

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
  inline static void setNoteTint(std::uint8_t index, Color color) noexcept {
    if (index < get().guitarBassColors.size()) {
      color.a = 255;
      get().guitarBassColors[index] = color;
    }
  }
  inline static const std::string &getSerialPort() noexcept {
    return get().serialPort;
  }
  inline static std::uint32_t getSerialBaudRate() noexcept {
    return get().serialBaudRate;
  }
  static std::vector<std::string> getAvailableSerialPorts();
  inline static int getMasterVolume() noexcept { return get().masterVolume; }
  static void setMasterVolume(int percent) noexcept;
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
  inline static bool saveSettings() noexcept { return get().iSaveSettings(); }
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

  template <InstrumentType Type>
    requires CustomType<Type>
  inline static void createCustomInstrument(std::string name,
                                            std::uint8_t n1 = 0,
                                            std::uint8_t n2 = 0) {
    return get().iCreateCustomInstrument<Type>(std::move(name), n1, n2);
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
  bool iSaveSettings() noexcept;
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

  template <InstrumentType Type>
    requires CustomType<Type>
  void iCreateCustomInstrument(std::string &&name, std::uint8_t n1,
                               std::uint8_t n2);

  static constexpr std::string fileName{"settings.bin"};

  std::array<Color, 6> guitarBassColors{RED,   ORANGE, YELLOW,
                                        GREEN, BLUE,   PURPLE};
  std::string serialPort;
  std::uint32_t serialBaudRate{115200};
  int masterVolume{100};
};

template <InstrumentType Type>
  requires CustomType<Type>
void Settings::iCreateCustomInstrument(std::string &&name, std::uint8_t n1,
                                       std::uint8_t n2) {
  if constexpr (Type != InstrumentType::Custom_3) {
    if (!n1 || !n2) {
      return;
    }
  }

  std::filesystem::path instruments("Instruments/");

  std::uint16_t count{1};
  for (auto const &_ : std::filesystem::directory_iterator{instruments}) {
    ++count;
  }

  std::ofstream file(instruments.string().append("/").append(
                         std::to_string(count).append(".lua")),
                     std::ios_base::trunc);

  file << "---@type string\n";
  file << "name = \"" << name << "\"\n\n";

  constexpr std::string_view TypeText = []<InstrumentType T> consteval -> auto {
    static constexpr auto enumerators =
        std::define_static_array(std::meta::enumerators_of(^^InstrumentType));

    template for (constexpr auto enumerator : enumerators) {
      if (std::meta::extract<InstrumentType>(enumerator) == T) {
        return std::meta::identifier_of(enumerator);
      }
    }
    std::unreachable();
  }.template operator()<Type>();

  file << "-- Custom_1 / Custom_2 / Custom_3\n";

  file << "Type = " << TypeText << "\n\n";

  file << "-- For " << TypeText << " Only!\n";

  if constexpr (Type != InstrumentType::Custom_3) {

    const InstrumentComposition<Type> composition{n1, n2};

    static constexpr auto members =
        std::define_static_array(std::meta::nonstatic_data_members_of(
            ^^InstrumentComposition<Type>,
            std::meta::access_context::current()));

    file << "Composition = {\n";

    template for (constexpr auto member : members) {
      file << '\t' << std::meta::identifier_of(member) << " = "
           << static_cast<std::uint32_t>(composition.[:member:]) << ",\n";
    }

    file << "}\n";

  } else {

    file << '\n';

    static constexpr auto Difficulties =
        std::define_static_array(std::meta::enumerators_of(^^Difficulty));

    template for (constexpr auto Dif : Difficulties) {
      constexpr auto DifIdentifier = std::meta::identifier_of(Dif);

      file << "-- For " << DifIdentifier << " Difficulty\n";
      file << "---@param note integer\n";
      file << "---@return integer\n";
      file << "function Play" << DifIdentifier
           << "(note)\n\treturn note\nend\n";
      file << '\n';
    }

    file << '\n';

    file << "-- drawNote(position, color) to draw a note\n";

    file << "---@param incomingNote integer\n";
    file << "---@return nil\n";
    file << "function Draw(incomingNote)\n\t\nend\n";
  }
}

} // namespace bh
