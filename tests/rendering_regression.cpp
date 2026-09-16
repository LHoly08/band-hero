#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "core/StateStack.hpp"
#include "states/GameState.hpp"
#include "states/GamemodeState.hpp"
#include "states/MainMenuState.hpp"
#include "states/PlayerSelectState.hpp"
#include "states/SettingsState.hpp"

namespace {
void check(bool condition, const char *message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}
bool nearlyEqual(float a, float b) { return std::abs(a - b) < 0.01f; }

std::string applicationDirectory;
std::map<unsigned, Texture2D> liveTextures;
unsigned nextTexture = 10;
int textureLoads{}, textureUnloads{}, settingsSaves{}, randomCalls{};
int screenWidth = 1920, screenHeight = 1080;
Vector2 mouse{};
bool clicked{};
struct TextureDraw {
  Rectangle source, dest;
};
struct TextDraw {
  std::string text;
  Rectangle bounds;
};
std::vector<TextureDraw> textureDraws;
std::vector<TextDraw> textDraws;
struct Transform {
  float x{}, y{}, sx{1.f}, sy{1.f};
} transform;
std::vector<Transform> transforms;

void beginFrame() {
  textureDraws.clear();
  textDraws.clear();
}
void draw(bh::StateStack &stack, std::size_t buttons) {
  beginFrame();
  stack.draw();
  check(textureDraws.size() == buttons, "expected visible buttons only");
  for (const auto &label : textDraws) {
    check(label.bounds.x >= 0 && label.bounds.y >= 0 &&
              label.bounds.x + label.bounds.width <= screenWidth + 0.01f &&
              label.bounds.y + label.bounds.height <= screenHeight + 0.01f,
          "label must stay on screen");
  }
}
void click(bh::StateStack &stack, Vector2 referencePosition) {
  mouse = bh::scaledSize(referencePosition);
  clicked = true;
  stack.events();
  clicked = false;
  stack.act();
}
bool hasText(std::string_view expected) {
  return std::ranges::any_of(textDraws, [expected](const auto &draw) {
    return draw.text == expected;
  });
}
} // namespace

// Match raylib's by-value validity check, including stale handles after unload.
bool IsTextureValid(Texture2D t) {
  return t.id > 0 && t.width > 0 && t.height > 0 && t.mipmaps > 0 &&
         t.format > 0;
}
Texture2D LoadTexture(const char *path) {
  if (!std::filesystem::is_regular_file(path)) {
    std::cerr << "Missing texture: " << path << '\n';
  }
  check(std::filesystem::is_regular_file(path),
        "requested texture asset must exist");
  const bool buttons = std::filesystem::path(path).filename() == "Buttons.png";
  Texture2D t{nextTexture++, buttons ? 631 : 300, buttons ? 200 : 150, 1, 7};
  liveTextures.emplace(t.id, t);
  ++textureLoads;
  return t;
}
void UnloadTexture(Texture2D t) {
  check(liveTextures.erase(t.id) == 1, "texture must be unloaded exactly once");
  ++textureUnloads;
}
void DrawTexturePro(Texture2D t, Rectangle source, Rectangle dest, Vector2,
                    float, Color) {
  check(liveTextures.contains(t.id), "draw must use a live texture");
  check(source.x >= 0 && source.y >= 0 && source.x + source.width <= t.width &&
            source.y + source.height <= t.height,
        "sprite rectangle must fit atlas");
  textureDraws.push_back({source, dest});
}
const char *GetApplicationDirectory() { return applicationDirectory.c_str(); }
int GetScreenWidth() { return screenWidth; }
int GetScreenHeight() { return screenHeight; }
// Deliberately simulate a high-DPI framebuffer.
int GetRenderWidth() { return screenWidth * 2; }
int GetRenderHeight() { return screenHeight * 2; }
Vector2 GetMousePosition() { return mouse; }
bool IsMouseButtonPressed(int) { return clicked; }
int GetRandomValue(int min, int) { return min + (++randomCalls % 2); }
Font GetFontDefault() {
  Font font{};
  font.baseSize = 10;
  font.texture = {1, 128, 128, 1, 7};
  return font;
}
Font LoadFont(const char *path) {
  check(std::filesystem::is_regular_file(path), "font asset must exist");
  Font font{};
  font.baseSize = 32;
  font.texture = {2, 128, 128, 1, 7};
  return font;
}
void UnloadFont(Font font) {
  check(font.texture.id != 0 && font.texture.id != 1,
        "do not unload default font");
}
Vector2 MeasureTextEx(Font, const char *text, float size, float spacing) {
  const auto count = std::string_view(text).size();
  return {count ? count * size * 0.6f + (count - 1) * spacing : 0.f, size};
}
void DrawTextEx(Font font, const char *text, Vector2 pos, float size,
                float spacing, Color) {
  const auto bounds = MeasureTextEx(font, text, size, spacing);
  textDraws.push_back(
      {text,
       {transform.x + pos.x * transform.sx, transform.y + pos.y * transform.sy,
        bounds.x * transform.sx, bounds.y * transform.sy}});
}
void rlPushMatrix() { transforms.push_back(transform); }
void rlPopMatrix() {
  transform = transforms.back();
  transforms.pop_back();
}
void rlTranslatef(float x, float y, float) {
  transform.x += x * transform.sx;
  transform.y += y * transform.sy;
}
void rlScalef(float x, float y, float) {
  transform.sx *= x;
  transform.sy *= y;
}

// Keep regression tests independent of hardware and user configuration.
serialib::serialib() = default;
serialib::~serialib() = default;
char serialib::openDevice(const char *, unsigned int, SerialDataBits,
                          SerialParity, SerialStopBits) {
  return 1;
}
void serialib::closeDevice() {}
int serialib::readBytes(void *, unsigned int, unsigned int, unsigned int) {
  return 0;
}
namespace bh {
Settings::Settings() = default;
void Settings::iSaveSettings() noexcept { ++settingsSaves; }
} // namespace bh

namespace {
void textureLifetime() {
  using namespace bh;
  const auto before = textureLoads;
  ResourceManager::loadTextures<Textures::UI, Textures::UI>();
  check(textureLoads == before + 1,
        "forward texture groups and avoid duplicate loads");
  ResourceManager::loadTexture<Textures::Gameplay::Notes>();
  check(textureLoads == before + 2, "load individual textures from a group");
  ResourceManager::loadTextures<Textures::UI>();
  check(textureLoads == before + 2, "repeated load must be idempotent");
  ResourceManager::unloadTextures<Textures::UI>();
  ResourceManager::unloadTextures<Textures::UI>();
  ResourceManager::loadTextures<Textures::UI>();
  check(textureLoads == before + 3, "cleared handle must allow reload");
  ResourceManager::unloadTextures<Textures::UI, Textures::UI>();
  ResourceManager::unloadTexture<Textures::Gameplay::Notes>();
  check(liveTextures.empty(), "all requested texture groups must unload");
}

void screens() {
  using namespace bh;
  StateStack stack;
  stack.push<MainMenuState>();
  check(liveTextures.empty(),
        "queued state must not load textures before entry");
  stack.act();
  for (int iteration = 0; iteration < 3; ++iteration) {
    draw(stack, 3);
    check(hasText("Play") && hasText("Quit"), "main menu labels");
    click(stack, {1050, 350});
    draw(stack, 3);
    check(hasText("Save") && hasText("Main Menu") && hasText("Defaults"),
          "all settings buttons and labels must render");
    check(settingsSaves == 0, "drawing must not write settings");
    click(stack, {1100, 400});
    draw(stack, 3);
    check(hasText("Play"), "return from settings");
    click(stack, {200, 150});
    draw(stack, 3);
    check(hasText("Local") && hasText("LAN") && hasText("Online"),
          "mode labels");
    click(stack, {200, 150});
    draw(stack, 4);
    check(hasText("Choose Instruments") && hasText("1"), "player count stage");
    click(stack, {550, 550});
    draw(stack, 4);
    check(hasText("2"), "player count input aligns with rendering");
    click(stack, {200, 150});
    draw(stack, 3);
    check(hasText("Go Back") && hasText("Start"), "instrument selection stage");
    // This part of Start overlaps the hidden player-count increment button.
    click(stack, {850, 650});
    draw(stack, 1);
    check(hasText("Cancel"), "instrument testing stage");
    click(stack, {1100, 750});
    draw(stack, 3);
    click(stack, {1500, 100});
    draw(stack, 3);
    check(hasText("Play"), "return from player selection");
  }
  stack.push<SettingsState>();
  stack.act();
  draw(stack, 3);
  stack.pop();
  stack.act();
  draw(stack, 3);
  check(hasText("Play"), "resumed state reloads its textures");
  stack.replace<GamemodeState>();
  stack.clear();
  stack.act();
  draw(stack, 0);
  check(liveTextures.empty(),
        "clear releases active and discards queued states");
}

void buttonText() {
  using namespace bh;
  ResourceManager::loadTextures<Textures::UI>();
  const std::string backing = "Choose Instruments -- not part of the label";
  const std::string_view label(backing.data(), 18);
  Button button({100, 100}, {0, 0, 500, 200}, label);
  beginFrame();
  button.draw();
  check(textDraws.size() == 1 && hasText("Choose Instruments"),
        "bounded string_view label");
  const auto rect = textureDraws.back().dest;
  const auto text = textDraws.back().bounds;
  check(text.x >= rect.x &&
            text.x + text.width <= rect.x + rect.width + 0.01f &&
            text.y >= rect.y && text.y + text.height <= rect.y + rect.height,
        "long button label must fit its background");
  check(button.pressed(scaledSize(Vector2{350, 200})),
        "scaled button hit test");
  check(!button.pressed(scaledSize(Vector2{650, 200})),
        "outside button hit test");
  check(nearlyEqual(rect.x, 100.f * screenWidth / 1920.f),
        "use logical screen dimensions");
  button.changeText(label.substr(0, 6));
  beginFrame();
  button.draw<WHITE, true, 64, TextAlign::Center>();
  check(hasText("Choose"), "changeText honors string_view length");
  check(nearlyEqual(
            textDraws.back().bounds.x + textDraws.back().bounds.width / 2.f,
            textureDraws.back().dest.x + textureDraws.back().dest.width / 2.f),
        "centered text uses the same measurement as drawing");
  beginFrame();
  ResourceManager::drawText<Fonts_t::Default>(label.substr(0, 6), {0, 0});
  check(hasText("Choose"), "drawText honors string_view length");
  check(nearlyEqual(
            ResourceManager::measureText<Fonts_t::Default>(label.substr(0, 6)),
            MeasureTextEx(GetFontDefault(), "Choose", 64, 5).x),
        "measureText honors string_view length");
  ResourceManager::unloadTextures<Textures::UI>();
}

struct FakePlayer final : bh::PlayerBase {
  int &draws;
  explicit FakePlayer(int &count) : draws(count) {}
  void draw() const noexcept override { ++draws; }
  void update(float) noexcept override {}
  void play(std::uint32_t) noexcept override {}
  void updateInstrumentSpeed(const float &) noexcept override {}
};
template <std::uint8_t N> void gameplay() {
  using namespace bh;
  StateStack stack;
  stack.push<MainMenuState>();
  stack.act();
  std::array<std::unique_ptr<PlayerBase>, N> players;
  int playerDraws{};
  for (auto &player : players)
    player = std::make_unique<FakePlayer>(playerDraws);
  stack.push<GameState<N>>(std::move(players));
  stack.act();
  draw(stack, 0);
  check(playerDraws == N,
        "gameplay draws every player without underlying menu");
  stack.pop();
  stack.act();
  draw(stack, 3);
  stack.clear();
}

// Expose only the protected note buffer through a pointer to member, so actual
// final instrument classes can be tested without adding test APIs to the game.
template <bh::InstrumentType Type, bh::Difficulty Dif>
struct SeedNotes : bh::Instrument<Type, Dif> {
  static void add(bh::Instrument<Type, Dif> &instrument, bh::Note<Type> note) {
    auto buffer = &SeedNotes::m_activeBuffer;
    (instrument.*buffer).push_back(note);
  }
};
void notes() {
  using namespace bh;
  ResourceManager::loadTexture<Textures::Gameplay::Notes>();
  std::uint32_t count{};
  Guitar<Difficulty::Hard> guitar(count);
  Bass<Difficulty::Hard> bass(count);
  SeedNotes<InstrumentType::Guitar, Difficulty::Hard>::add(guitar, {2, 400, 1});
  SeedNotes<InstrumentType::Bass, Difficulty::Hard>::add(bass, {8, 400, 0});
  const int randomBefore = randomCalls;
  for (int frame = 0; frame < 5; ++frame) {
    beginFrame();
    guitar.draw(0);
    bass.draw(0);
    check(textureDraws.size() == 2,
          "all fret bits must be decoded for rendering");
    check(textureDraws[0].source.x == 150 && textureDraws[1].source.x == 0,
          "note sprite must remain stable across frames");
  }
  check(randomCalls == randomBefore, "drawing must not consume randomness");
  Custom<InstrumentType::Custom_2, Difficulty::Hard> custom(count, "test",
                                                            {2, 6});
  SeedNotes<InstrumentType::Custom_2, Difficulty::Hard>::add(custom,
                                                             {1u << 5, 400, 0});
  beginFrame();
  custom.draw(0);
  check(textureDraws.size() == 1,
        "hard custom instruments render all hard-mode bits");
  ResourceManager::unloadTexture<Textures::Gameplay::Notes>();
}
} // namespace

int main(int, char **argv) {
  applicationDirectory =
      std::filesystem::absolute(argv[0]).parent_path().string();
  textureLifetime();
  for (const auto size : {Vector2{1920, 1080}, Vector2{1280, 720},
                          Vector2{960, 540}, Vector2{2560, 1440}}) {
    screenWidth = static_cast<int>(size.x);
    screenHeight = static_cast<int>(size.y);
    screens();
    buttonText();
  }
  gameplay<1>();
  gameplay<2>();
  gameplay<3>();
  gameplay<4>();
  notes();
  bh::ResourceManager::loadTextures<bh::Textures::UI>();
  bh::ResourceManager::unload();
  bh::ResourceManager::unload();
  check(liveTextures.empty() && textureLoads == textureUnloads,
        "shutdown must release each texture once");
  check(transforms.empty(), "text transforms must remain balanced");
  std::cout << "Rendering regression checks passed\n";
}
