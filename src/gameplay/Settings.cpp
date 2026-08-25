#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

#include "gameplay/Settings.hpp"
#include "serial/serialib.h"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace bh {
namespace {

std::vector<std::string> serialPortCandidates() {
  std::vector<std::string> ports;

#if defined(_WIN32)
  for (unsigned int number = 1; number <= 256; ++number) {
    const std::string name = "COM" + std::to_string(number);
    std::array<char, 1024> target{};
    if (QueryDosDeviceA(name.c_str(), target.data(), target.size()) != 0) {
      ports.emplace_back("\\\\.\\" + name);
    }
  }
#elif defined(__linux__) || defined(__APPLE__)
  std::error_code error;
  for (const auto &entry :
       std::filesystem::directory_iterator{"/dev", error}) {
    const std::string name = entry.path().filename().string();
#if defined(__APPLE__)
    const bool isSerialPort = name.starts_with("cu.");
#else
    constexpr std::array prefixes{"ttyACM", "ttyUSB", "ttyS", "ttyAMA",
                                  "ttyTHS", "rfcomm"};
    const bool isSerialPort =
        std::ranges::any_of(prefixes, [&name](std::string_view prefix) {
          return name.starts_with(prefix);
        });
#endif
    if (isSerialPort) {
      ports.push_back(entry.path().string());
    }
  }
#endif

  std::ranges::sort(ports);
  return ports;
}

} // namespace

Settings::Settings() { iLoadSettings(); }

std::vector<std::string> Settings::getAvailableSerialPorts() {
  return serialPortCandidates();
}

void Settings::iDefaultSettings() noexcept {
  guitarBassColors = {RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE};
  serialPort.clear();
  serialBaudRate = 115200;
  iSaveSettings();
}

void Settings::iLoadSettings() noexcept {
  std::ifstream file{fileName, std::ios::binary};
  if (!file) {
    iDefaultSettings();
    iDetectSerialPort();
    iSaveSettings();
    return;
  }

  for (Color &color : guitarBassColors) {
    std::array<unsigned char, 4> rgba{};
    if (!file.read(reinterpret_cast<char *>(rgba.data()), rgba.size())) {
      iDefaultSettings();
      iDetectSerialPort();
      iSaveSettings();
      return;
    }
    color = {rgba[0], rgba[1], rgba[2], rgba[3]};
  }

  std::array<unsigned char, 2> encodedLength{};
  if (file.read(reinterpret_cast<char *>(encodedLength.data()),
                encodedLength.size())) {
    const std::size_t length = encodedLength[0] |
                               (static_cast<std::size_t>(encodedLength[1]) << 8);
    serialPort.resize(length);
    if (!file.read(serialPort.data(), static_cast<std::streamsize>(length))) {
      serialPort.clear();
    }
  }

  std::array<unsigned char, 4> encodedBaudRate{};
  if (file.read(reinterpret_cast<char *>(encodedBaudRate.data()),
                encodedBaudRate.size())) {
    const std::uint32_t baudRate =
        encodedBaudRate[0] |
        (static_cast<std::uint32_t>(encodedBaudRate[1]) << 8) |
        (static_cast<std::uint32_t>(encodedBaudRate[2]) << 16) |
        (static_cast<std::uint32_t>(encodedBaudRate[3]) << 24);
    iSetSerialBaudRate(baudRate);
  }

}

void Settings::iSaveSettings() noexcept {
  std::ofstream file{fileName, std::ios::binary | std::ios::trunc};
  if (!file) {
    return;
  }

  for (const Color color : guitarBassColors) {
    const std::array<unsigned char, 4> rgba{color.r, color.g, color.b, color.a};
    file.write(reinterpret_cast<const char *>(rgba.data()), rgba.size());
  }

  const std::size_t length = std::min<std::size_t>(serialPort.size(), 0xffff);
  const std::array<unsigned char, 2> encodedLength{
      static_cast<unsigned char>(length & 0xff),
      static_cast<unsigned char>((length >> 8) & 0xff)};
  file.write(reinterpret_cast<const char *>(encodedLength.data()),
             encodedLength.size());
  file.write(serialPort.data(), static_cast<std::streamsize>(length));

  const std::array<unsigned char, 4> encodedBaudRate{
      static_cast<unsigned char>(serialBaudRate & 0xff),
      static_cast<unsigned char>((serialBaudRate >> 8) & 0xff),
      static_cast<unsigned char>((serialBaudRate >> 16) & 0xff),
      static_cast<unsigned char>((serialBaudRate >> 24) & 0xff)};
  file.write(reinterpret_cast<const char *>(encodedBaudRate.data()),
             encodedBaudRate.size());
}

bool Settings::iDetectSerialPort() noexcept {
  try {
    serialPort.clear();
    for (const std::string &candidate : serialPortCandidates()) {
      serialib device;
      if (device.openDevice(candidate.c_str(), serialBaudRate) == 1) {
        serialPort = candidate;
        device.closeDevice();
        return true;
      }
    }
  } catch (...) {
    serialPort.clear();
  }
  return false;
}

bool Settings::iSetSerialBaudRate(std::uint32_t baudRate) noexcept {
  if (!std::ranges::contains(supportedBaudRates, baudRate)) {
    return false;
  }
  serialBaudRate = baudRate;
  return true;
}

} // namespace bh
