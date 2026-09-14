#include <algorithm>
#include <cctype>
#include <fstream>
#include <ranges>
#include <string_view>

#include "core/ResourceManager.hpp"

#include "raylib.h"

namespace bh {
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

          if (std::ifstream(path).is_open()) {

            return LoadFont(path.data());

          } else {

            std::ranges::transform(
                path.end() - 3, path.end(), path.end() - 3,
                [](char c) -> char { return std::tolower(c); });

            return LoadFont(path.data());
          }
        });
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
