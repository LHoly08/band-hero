#pragma once
#include <vector>
#include <string>
#include "ui/settings/Controls.hpp"

namespace bh {
class GeneralSettings {
public:
  void onEnter();
  void draw() const;
  bool events();
  void reset() { m_volume.reset(); }
private:
  void refreshSerialPorts();
  settings_ui::Slider m_volume{{1130, 349, 530, 10}};
  std::vector<Vector2> m_resolutions;
  std::vector<int> m_frameRates;
  std::vector<std::string> m_serialPorts;
  int m_refreshRate{};
};
} // namespace bh
