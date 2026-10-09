#pragma once

#include <vector>

#include "raylib.h"

#include "core/ResourceManager.hpp"

namespace bh {

enum class Achievements : std::uint8_t {
  BandTogether = 0,
  BassMastery,
  Bass,
  Completion,
  Drums,
  DrumsMastery,
  Easy,
  FirstGig,
  FourPlayer,
  FullCombo,
  Guitar,
  GuitarMastery,
  Hard,
  LAN,
  Online,
  Setlist,
  Streak,
  Medium,
};

class AchievementsManager final {
public:
  inline static AchievementsManager &get() {
    static AchievementsManager s_instance{};
    return s_instance;
  }
  AchievementsManager(const AchievementsManager &) = delete;
  void operator=(const AchievementsManager &) = delete;
  AchievementsManager(AchievementsManager &&) = delete;
  void operator=(AchievementsManager &&) = delete;

  ~AchievementsManager() = default;

  static inline void drawMedal(Achievements Medal,
                               const Vector2 &position) noexcept {
    const std::uint8_t medalValue = static_cast<std::uint8_t>(Medal);

    const std::uint8_t positionX = medalValue % 9;
    const std::uint8_t positionY = medalValue / 9;

    const Rectangle Source = {.x = 200.f * positionX,
                              .y = 200.f * positionY,
                              .width = 200,
                              .height = 200};

    ResourceManager::drawImage<Textures::Achievements::Medals>(Source, position,
                                                               WHITE);
  }

  static inline std::vector<Achievements> getEarnedMedals() {
    return get().iGetEarnedMedals();
  }

private:
  AchievementsManager();

  static constexpr std::string_view AchievementsPath{"medals.bin"};
  inline consteval std::uint8_t medalBytes() {
    return std::meta::enumerators_of(^^Achievements).size() / 8;
  };

  std::vector<Achievements> iGetEarnedMedals();
};

} // namespace bh
