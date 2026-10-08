#include <array>
#include <fstream>

#include "medals/AchievementsManager.hpp"

namespace bh {

AchievementsManager::AchievementsManager() {
  std::fstream achievementsFile(AchievementsPath.data(), std::ios::in);
  if (!achievementsFile.is_open()) {
    achievementsFile.close();

    std::ofstream file(AchievementsPath.data(),
                       std::ios::binary | std::ios::trunc);

    std::array<char, medalBytes()> stream{};
    file.write(stream.data(), medalBytes());
  }
}

std::vector<Achievements> AchievementsManager::iGetEarnedMedals() {
  std::ifstream file(AchievementsPath.data(), std::ios::binary);

  std::array<std::uint8_t, medalBytes()> buffer;
  file.read(reinterpret_cast<char *>(buffer.data()), medalBytes());

  return {};
}

} // namespace bh
