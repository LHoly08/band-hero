#include <algorithm>
#include <cctype>
#include <fstream>
#include <ranges>

#include "core/ResourceManager.hpp"

#include "raylib.h"

namespace bh {
ResourceManager::ResourceManager()
    : m_textures(Textures::size()), m_fonts(Fonts::size()) {

  {
    m_fonts.front() = GetFontDefault();
    constexpr auto fontFiles = Fonts::files();

    for (std::uint8_t i{1}; i < Fonts::size(); ++i) {

      std::string path("assets/font/");
      path.append(fontFiles[i - 1]).append(".TTF");

      if (std::ifstream(path).is_open()) {

        m_fonts[i] = LoadFont(path.data());

      } else {

        std::ranges::transform(path.end() - 3, path.end(), path.end() - 3,
                               [](char c) -> char { return std::tolower(c); });

        m_fonts[i] = LoadFont(path.data());
      }
    }
  }
  {
    constexpr auto textureFiles = Textures::files();
    std::transform(textureFiles.begin(), textureFiles.end(), m_textures.begin(),
                   [](const char *const fileName) -> Texture2D {
                     return LoadTexture(fileName);
                   });
  }
}

void ResourceManager::iUnload() noexcept {

  for (auto &t : m_textures) {
    UnloadTexture(t);
  }
  for (auto &f : m_fonts | std::ranges::views::drop(1)) {
    UnloadFont(f);
  }
}

} // namespace bh
