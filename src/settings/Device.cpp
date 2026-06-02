#include <filesystem>

#include "settings/Device.hpp"

#include "serial/serialib.h"

namespace bh {

std::string Device::IportName() const noexcept {
  if (m_port < m_availablePorts.size()) {
    return m_availablePorts[m_port];
  }
  return m_availablePorts.back();
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
    // Windows requires the "\\\\.\\COM" prefix for port numbers greater than 9
    std::string portPath = "\\\\.\\COM" + std::to_string(i);

    // Try opening the port at a default baud rate
    if (testSerial.openDevice(portPath.c_str(), 115200) == 1) {
      testSerial.closeDevice();
      // Store the user-friendly name "COMX" for your game UI
      m_availablePorts.push_back("COM" + std::to_string(i));
    }
  }

#elif defined(__APPLE__)

  std::string basePaths[] = {"/dev/cu.usbmodem", "/dev/cu.usbserial-"};

  for (const auto &base : basePaths) {
    for (int i = 0; i < 30; ++i) {
      // Check short variations (e.g., /dev/cu.usbmodem1)
      std::string testPath = base + std::to_string(i);
      if (testSerial.openDevice(testPath.c_str(), 115200) == 1) {
        testSerial.closeDevice();
        m_availablePorts.push_back(testPath);
      }

      // Check standard macOS location ID variations (e.g.,
      // /dev/cu.usbmodem1410)
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

      // Filter for common Linux USB-to-Serial chips (ACM and USB)
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
