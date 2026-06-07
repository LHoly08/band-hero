
#include "settings/Device.hpp"

#include "serial/serialib.h"

namespace bh {

std::optional<std::string> Device::IportName() const noexcept {
  if (m_availablePorts.empty()) {
    return std::nullopt;
  } else if (m_port >= m_availablePorts.size()) {
    return m_availablePorts.back();
  }
  return m_availablePorts[m_port];
}

void Device::Iinit() noexcept {
  Iscan();
  m_port = 0;
  m_baud = 115200;
}

void Device::Iscan() noexcept {

  m_availablePorts.clear();
  serialib testSerial;

#if defined(_WIN32)

  for (int i = 1; i <= 32; ++i) {
    std::string portPath = "COM" + std::to_string(i);

    if (testSerial.openDevice(portPath.c_str(), 115200) == 1) {

      testSerial.closeDevice();
      m_availablePorts.push_back("COM" + std::to_string(i));
    }
  }

#elif defined(__APPLE__)

  std::string basePaths[] = {"/dev/cu.usbmodem", "/dev/cu.usbserial-"};

  for (const auto &base : basePaths) {
    for (int i = 0; i < 30; ++i) {

      std::string testPath = base + std::to_string(i);
      if (testSerial.openDevice(testPath.c_str(), 115200) == 1) {
        testSerial.closeDevice();
        m_availablePorts.push_back(testPath);
      }

      std::string longTestPath = base + std::to_string(1400 + i);
      if (testSerial.openDevice(longTestPath.c_str(), 115200) == 1) {
        testSerial.closeDevice();
        m_availablePorts.push_back(longTestPath);
      }
    }
  }

#else

  std::string devPath = "/dev/";
  if (std::filesystem::exists(devPath) &&
      std::filesystem::is_directory(devPath)) {
    for (const auto &entry : std::filesystem::directory_iterator(devPath)) {
      std::string pathString = entry.path().string();

      if (pathString.find("ttyUSB") != std::string::npos ||
          pathString.find("ttyACM") != std::string::npos) {

        if (testSerial.openDevice(pathString.c_str(), 115200) == 1) {
          testSerial.closeDevice();
          m_availablePorts.push_back(pathString);
        }
      }
    }
  }

#endif
}

} // namespace bh
