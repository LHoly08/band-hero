#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <string_view>

#include "core/ResourceManager.hpp"

#include "raylib.h"

namespace bh {
std::string ResourceManager::assetPath(std::string_view relativePath) {
  // Installed builds keep assets beside the executable, regardless of cwd.
  const auto installedPath =
      std::filesystem::path(GetApplicationDirectory()) / relativePath;
  if (std::filesystem::is_regular_file(installedPath)) {
    return installedPath.string();
  }
  return std::string(relativePath);
}

ResourceManager::ResourceManager()
    : m_textures(Textures::size()), m_fonts(Fonts::size()) {

  {
    m_fonts.front() = GetFontDefault();
    constexpr auto fontFiles = Fonts::files();

    std::ranges::transform(
        fontFiles.begin(), fontFiles.end(), m_fonts.begin() + 1,
        [](std::string_view s) -> Font {
          std::string path("assets/font/");
          path.append(s).append(".TTF");

          if (std::ifstream(assetPath(path)).is_open()) {

            return LoadFont(assetPath(path).c_str());

          } else {

            std::ranges::transform(
                path.end() - 3, path.end(), path.end() - 3,
                [](char c) -> char { return std::tolower(c); });

            return LoadFont(assetPath(path).c_str());
          }
        });
  }
}

void ResourceManager::iLoadTexture(std::size_t index) noexcept {
  constexpr auto files = Textures::files();
  if (!IsTextureValid(m_textures[index])) {
    m_textures[index] = LoadTexture(assetPath(files[index]).c_str());
  }
}

void ResourceManager::iUnloadTexture(std::size_t index) noexcept {
  if (IsTextureValid(m_textures[index])) {
    UnloadTexture(m_textures[index]);
  }
  m_textures[index] = {};
}

void ResourceManager::iUnload() noexcept {

  for (std::size_t i{}; i < m_textures.size(); ++i) {
    iUnloadTexture(i);
  }
  for (auto &f : m_fonts | std::ranges::views::drop(1)) {
    if (f.texture.id != 0 && f.texture.id != GetFontDefault().texture.id) {
      UnloadFont(f);
    }
    f = {};
  }
}

} // namespace bh
