#pragma once

#include <algorithm>
#include <cstdint>
#include <meta>
#include <ranges>
#include <vector>

#include "raylib.h"
#include "rlgl.h"

#include "core/Scale.hpp"

namespace bh {

struct Textures {
  enum class Type : std::uint8_t {
    Buttons = 0,
    Notes,
  };

  static consteval std::uint8_t size() noexcept {

    constexpr auto max = []() -> std::uint8_t {
      std::vector<std::meta::info> n =
          std::meta::enumerators_of(^^Textures::Type);
      std::vector<std::uint8_t> v(n.size());

      std::ranges::transform(
          n.begin(), n.end(), v.begin(), [](std::meta::info e) -> std::uint8_t {
            return (std::uint8_t)std::meta::extract<Textures::Type>(e);
          });
      return *std::max_element(v.begin(), v.end());
    };

    return max() + 1;
  }
};

struct Fonts {
  enum class Type : std::uint8_t {
    Default = 0,
    Buttons,
  };

  static consteval std::uint8_t size() noexcept {

    std::vector<std::meta::info> enumData =
        std::meta::enumerators_of(^^Fonts::Type);

    std::vector<std::uint8_t> vals(enumData.size());

    std::ranges::transform(
        enumData.begin(), enumData.end(), vals.begin(),
        [](std::meta::info e) -> std::uint8_t {
          return (std::uint8_t)std::meta::extract<Fonts::Type>(e);
        });

    return (*std::max_element(vals.begin(), vals.end())) + 1;
  }

  static consteval auto files() noexcept {

    std::array<std::string_view, Fonts::size() - 1> result{};

    auto enumData = std::meta::enumerators_of(^^Fonts::Type);

    for (const auto &e : enumData) {
      const std::uint8_t index =
          (std::uint8_t)std::meta::extract<Fonts::Type>(e);

      if (index != 0) {
        result[index - 1] = std::meta::identifier_of(e);
      }
    }

    return result;
  }
};

using Textures_t = Textures::Type;
using Fonts_t = Fonts::Type;

class ResourceManager final {
public:
  inline static ResourceManager &get() {
    static ResourceManager s_instance{};
    return s_instance;
  }

  ~ResourceManager() = default;
  inline static void unload() noexcept { return get().iUnload(); }

  ResourceManager(const ResourceManager &) = delete;
  void operator=(const ResourceManager &) = delete;
  ResourceManager(ResourceManager &&) = delete;
  void operator=(ResourceManager &&) = delete;

  // Like drawImage, pos and fontSize use the unscaled 1280x720 layout.
  template <Fonts_t FontType>
  inline static void drawText(const std::string_view text, const Vector2 &pos,
                              const std::uint32_t &fontSize = 64,
                              const Color &tint = BLACK) noexcept {
    return get().iDrawText<FontType>(text, pos, fontSize, tint);
  }

  template <Fonts_t FontType>
  // Return the unscaled layout width for positioning before drawText scales it.
  inline static float measureText(const std::string_view text,
                                  const std::uint32_t &fontSize = 64) noexcept {
    return get().iMeasureText<FontType>(text, fontSize);
  }

  template <Textures_t TextureType>
  inline static void drawImage(const Rectangle &rect, const Vector2 &pos,
                               const Color &tint = WHITE) noexcept {
    return get().iDrawImage<TextureType>(rect, pos, tint);
  }

private:
  template <Fonts_t FontType>
  void iDrawText(const std::string_view text, const Vector2 &pos,
                 const std::uint32_t &fontSize,
                 const Color &tint) const noexcept;

  template <Fonts_t FontType>
  float iMeasureText(const std::string_view text,
                     const std::uint32_t &fontSize) const noexcept;

  template <Textures_t TextureType>
  void iDrawImage(const Rectangle &rect, const Vector2 &pos,
                  const Color &tint) const noexcept;

  void iUnload() noexcept;

  ResourceManager();

  std::vector<Texture2D> m_textures;
  std::vector<Font> m_fonts;
};

template <Fonts_t FontType>
void ResourceManager::iDrawText(const std::string_view text, const Vector2 &pos,
                                const std::uint32_t &fontSize,
                                const Color &tint) const noexcept {

  const Vector2 scale = scaledSize(Vector2{1.f, 1.f});
  const Vector2 scaledPos = scaledSize(pos);
  rlPushMatrix();
  rlTranslatef(scaledPos.x, scaledPos.y, 0.f);
  rlScalef(scale.x, scale.y, 1.f);
  DrawTextEx(m_fonts[(std::uint8_t)FontType], text.data(), {0.f, 0.f},
             (float)fontSize, 5.f, tint);
  rlPopMatrix();
}

template <Fonts_t FontType>
float ResourceManager::iMeasureText(
    const std::string_view text, const std::uint32_t &fontSize) const noexcept {

  Vector2 res = MeasureTextEx(m_fonts[(std::uint8_t)FontType], text.data(),
                              (float)fontSize, 5.f);

  return res.x;
}

template <Textures_t TextureType>
void ResourceManager::iDrawImage(const Rectangle &rect, const Vector2 &pos,
                                 const Color &tint) const noexcept {

  const Vector2 scaledPos = scaledSize(pos);
  Rectangle dest{scaledPos.x, scaledPos.y,
                 scaledSize<float, ScreenAxis::X>(rect.width),
                 scaledSize<float, ScreenAxis::Y>(rect.height)};

  DrawTexturePro(m_textures[(std::uint8_t)TextureType], rect, dest, {0, 0}, 0,
                 tint);
}

} // namespace bh
