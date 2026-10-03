#pragma once

namespace bh {

// startup.bin retains its original two-byte FPS prefix for older installs.
class DisplaySettings {
public:
  static DisplaySettings &get();
  int fps{60};
  int width{1920};
  int height{1080};
  bool fullscreen{true};
  bool showFPS{false};

  void load();
  bool save() const;
  void apply() const;
};

} // namespace bh
