#pragma once
#include <array>
#include "ui/settings/Controls.hpp"

namespace bh {
class GameplaySettings {
public:
  void draw() const;
  bool events();
  void reset();
private:
  int m_note{};
  std::array<settings_ui::Slider, 3> m_channels{
      settings_ui::Slider{{850, 575, 760, 12}},
      settings_ui::Slider{{850, 675, 760, 12}},
      settings_ui::Slider{{850, 775, 760, 12}}};
};
} // namespace bh
