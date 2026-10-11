#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string_view>
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
  inline static constexpr std::size_t MedalCount =
      std::meta::enumerators_of(^^Achievements).size();
  inline static constexpr std::size_t MedalBytes = (MedalCount + 7) / 8;
  inline static constexpr std::uint32_t StreakTarget = 100;

  explicit AchievementsManager(std::filesystem::path path = "medals.bin");

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
    drawMedal(Medal, Rectangle{position.x, position.y, 200, 200});
  }

  static inline void drawMedal(Achievements medal, const Rectangle &destination,
                               Color tint = WHITE) noexcept {
    // Preserve the saved enum IDs; the sprite sheet uses a different order.
    static constexpr std::array<std::uint8_t, MedalCount> Cells{
        0, 2, 1, 3, 4, 5, 6, 7, 13, 8, 9, 10, 11, 12, 14, 15, 16, 17};
    const auto value = static_cast<std::uint8_t>(medal);
    if (value >= Cells.size()) {
      return;
    }
    const auto index = Cells[value];
    ResourceManager::drawImageRegionTo<Textures::Achievements::Medals>(
        {200.f * (index % 9), 200.f * (index / 9), 200, 200}, destination,
        tint);
  }

  static inline std::vector<Achievements> getEarnedMedals() {
    return get().earnedMedals();
  }

  static std::string_view name(Achievements medal) noexcept;
  static std::string_view description(Achievements medal) noexcept;
  bool isUnlocked(Achievements medal) const noexcept;
  std::vector<Achievements> earnedMedals() const;
  // Returns only newly unlocked medals; repeated awards are idempotent.
  std::vector<Achievements> unlock(std::span<const Achievements> medals);

private:
  std::filesystem::path m_path;
  std::array<std::uint8_t, MedalBytes> m_medals{};
};

} // namespace bh
