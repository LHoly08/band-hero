#pragma once

#include <optional>
#include <string>
#include <vector>

namespace bh {

class Device {
public:
  ~Device() = default;
  void operator=(const Device &) = delete;
  Device(const Device &) = delete;

  static Device &Get() noexcept {
    static Device instance;
    return instance;
  }

  static void Init() noexcept { return Get().Iinit(); }
  static void ScanPorts() noexcept { return Get().Iscan(); }

  template <bool Port> static void Configure(const size_t port) noexcept {
    if constexpr (Port) {
      return Get().Iconfigure<1>(port, 0);
    } else {
      return Get().Iconfigure<2>(0, port);
    }
  }
  static void Configure(const size_t port, const unsigned int baud) noexcept {
    return Get().Iconfigure<0>(port, baud);
  };

  static std::optional<std::string> PortName() noexcept {
    return Get().IportName();
  };
  static unsigned int BaudRate() noexcept { return Get().m_baud; };

private:
  Device() = default;
  void Iinit() noexcept;

  std::optional<std::string> IportName() const noexcept;
  void Iscan() noexcept;
  template <unsigned char Type>
  void Iconfigure(const size_t port, const unsigned int baud) noexcept;

  unsigned int m_baud{115200};
  size_t m_port{0};
  std::vector<std::string> m_availablePorts;
};

template <unsigned char Type>
void Device::Iconfigure(const size_t port, const unsigned int baud) noexcept {

  if constexpr (Type != 2) {
    if (port < m_availablePorts.size()) {
      m_port = port;
    }
    return;
  }

  if constexpr (Type != 1) {
    m_baud = baud;
    return;
  }
}
} // namespace bh
