#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <string_view>

#include "core/ResourceManager.hpp"

#include "raylib.h"

namespace bh {
bh::ResourceManager::TextureHandle::TextureHandle(std::size_t index) noexcept
    : m_index(index) {
  bh::ResourceManager::get().iLoadTexture(m_index);
}

bh::ResourceManager::TextureHandle::TextureHandle(const TextureHandle &other) noexcept
    : m_index(other.m_index) {
  if (m_index != Empty) {
    bh::ResourceManager::get().iLoadTexture(m_index);
  }
}

bh::ResourceManager::TextureHandle::TextureHandle(TextureHandle &&other) noexcept
    : m_index(std::exchange(other.m_index, Empty)) {}

bh::ResourceManager::TextureHandle &
bh::ResourceManager::TextureHandle::operator=(TextureHandle other) noexcept {
  std::swap(m_index, other.m_index);
  return *this;
}

bh::ResourceManager::TextureHandle::~TextureHandle() {
  if (m_index != Empty) {
    bh::ResourceManager::get().iUnloadTexture(m_index);
  }
}

::Texture2D bh::ResourceManager::TextureHandle::texture() const noexcept {
  return m_index == Empty ? ::Texture2D{} : bh::ResourceManager::get().m_textures[m_index];
}

std::string bh::ResourceManager::assetPath(std::string_view relativePath) {
  // Installed builds keep assets beside the executable, regardless of cwd.
  const auto installedPath =
      std::filesystem::path(::GetApplicationDirectory()) / relativePath;
  if (std::filesystem::is_regular_file(installedPath)) {
    return installedPath.string();
  }
  return std::string(relativePath);
}

bh::ResourceManager::ResourceManager()
    : m_textures(bh::Textures::size()), m_textureUsers(bh::Textures::size()),
      m_fonts(bh::Fonts::size()) {

  {
    m_fonts.front() = ::GetFontDefault();
    constexpr auto fontFiles = bh::Fonts::files();

    std::ranges::transform(
        fontFiles.begin(), fontFiles.end(), m_fonts.begin() + 1,
        [](std::string_view s) -> Font {
          std::string path("assets/font/");
          path.append(s).append(".TTF");

          if (!std::ifstream(assetPath(path)).is_open()) {
            std::ranges::transform(
                path.end() - 3, path.end(), path.end() - 3,
                [](char c) -> char { return std::tolower(c); });
          }
          // Rasterize above the normal button label size, then filter when scaled.
          ::Font font = ::LoadFontEx(assetPath(path).c_str(), 96, nullptr, 224);
          if (font.texture.id != 0) {
            ::SetTextureFilter(font.texture, ::TEXTURE_FILTER_BILINEAR);
          }
          return font;
        });
  }
}

void bh::ResourceManager::iLoadTexture(std::size_t index) noexcept {
  ++m_textureUsers[index];
  constexpr auto files = bh::Textures::files();
  if (!::IsTextureValid(m_textures[index])) {
    m_textures[index] = ::LoadTexture(assetPath(files[index]).c_str());
    if (::IsTextureValid(m_textures[index])) {
      // UI and scene artwork scale with the window; smooth fractional zooms
      // instead of enlarging individual texels with the default point filter.
      ::SetTextureFilter(m_textures[index], ::TEXTURE_FILTER_BILINEAR);
    }
  }
}

void bh::ResourceManager::iUnloadTexture(std::size_t index) noexcept {
  // Each state's load owns a reference, including states below an overlay.
  if (m_textureUsers[index] == 0 || --m_textureUsers[index] != 0) {
    return;
  }
  if (::IsTextureValid(m_textures[index])) {
    ::UnloadTexture(m_textures[index]);
  }
  m_textures[index] = {};
}

void bh::ResourceManager::iUnload() noexcept {

  for (std::size_t i{}; i < m_textures.size(); ++i) {
    // Final shutdown releases textures even if a caller retained a reference.
    m_textureUsers[i] = 1;
    iUnloadTexture(i);
  }
  for (auto &f : m_fonts | std::ranges::views::drop(1)) {
    if (f.texture.id != 0 && f.texture.id != ::GetFontDefault().texture.id) {
      ::UnloadFont(f);
    }
    f = {};
  }
}

} // namespace bh
