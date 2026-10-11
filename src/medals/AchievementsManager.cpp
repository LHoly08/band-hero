#include <fstream>

#include "medals/AchievementsManager.hpp"

namespace bh {

namespace {
constexpr std::array<std::string_view, AchievementsManager::MedalCount> Names{
    "Band Together", "Bass Mastery", "Bass", "Completion", "Drums",
    "Drums Mastery", "Easy", "First Gig", "Four Player", "Full Combo",
    "Guitar", "Guitar Mastery", "Hard", "LAN", "Online", "Setlist",
    "Streak", "Medium"};
constexpr std::array<std::string_view, AchievementsManager::MedalCount> Descriptions{
    "Finish a song with 2 or more players.",
    "Hit every bass note on Hard.",
    "Finish a song on bass.",
    "Hit 100% of the notes in a song.",
    "Finish a song on drums.",
    "Hit every drum note on Hard.",
    "Finish a song on Easy.",
    "Finish your first song.",
    "Finish a song with 4 players.",
    "Hit every note without incorrect presses.",
    "Finish a song on guitar.",
    "Hit every guitar note on Hard.",
    "Finish a song on Hard.",
    "Finish a LAN song (coming soon).",
    "Finish an online song (coming soon).",
    "Finish every song in a multi-song setlist.",
    "Hit 100 consecutive notes.",
    "Finish on Medium (coming soon)."};
}

AchievementsManager::AchievementsManager(std::filesystem::path path)
    : m_path(std::move(path)) {
  std::ifstream file(m_path, std::ios::binary);
  // Zero initialization also preserves compatibility with old two-byte saves.
  if (file) {
    file.read(reinterpret_cast<char *>(m_medals.data()), m_medals.size());
  }
}

std::string_view AchievementsManager::name(Achievements medal) noexcept {
  const auto index = static_cast<std::uint8_t>(medal);
  return index < Names.size() ? Names[index] : "Unknown";
}

std::string_view AchievementsManager::description(Achievements medal) noexcept {
  const auto index = static_cast<std::uint8_t>(medal);
  return index < Descriptions.size() ? Descriptions[index] : "";
}

bool AchievementsManager::isUnlocked(Achievements medal) const noexcept {
  const auto index = static_cast<std::uint8_t>(medal);
  return index < MedalCount && (m_medals[index / 8] & (1u << (index % 8)));
}

std::vector<Achievements> AchievementsManager::earnedMedals() const {
  std::vector<Achievements> earned;
  for (std::size_t i = 0; i < MedalCount; ++i) {
    const auto medal = static_cast<Achievements>(i);
    if (isUnlocked(medal)) {
      earned.push_back(medal);
    }
  }
  return earned;
}

std::vector<Achievements>
AchievementsManager::unlock(std::span<const Achievements> medals) {
  auto buffer = m_medals;
  std::vector<Achievements> unlocked;
  for (const auto medal : medals) {
    const auto index = static_cast<std::uint8_t>(medal);
    if (index >= MedalCount) {
      continue;
    }
    const auto mask = static_cast<std::uint8_t>(1u << (index % 8));
    if (!(buffer[index / 8] & mask)) {
      buffer[index / 8] |= mask;
      unlocked.push_back(medal);
    }
  }
  if (unlocked.empty()) {
    return unlocked;
  }
  std::ofstream file(m_path, std::ios::binary | std::ios::trunc);
  file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
  file.close();
  if (!file) {
    TraceLog(LOG_WARNING, "Could not save achievements: %s",
             m_path.string().c_str());
    return {};
  }
  m_medals = buffer;
  return unlocked;
}

} // namespace bh
