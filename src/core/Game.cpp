#include <SFML/Window/VideoMode.hpp>
#include <bit>
#include <cstdint>
#include <imgui-sfml.h>
#include <imgui.h>
#include <ios>
#include <iostream>

#include "core/Game.hpp"

#include "settings/Device.hpp"

#include <filesystem>
#include <fstream>

namespace bh {

Game::Game() noexcept
    : m_window(sf::VideoMode({1920, 1080}), "Band-Hero",
               sf::State::Fullscreen) {

  if (!ImGui::SFML::Init(m_window)) {
  }

  if (std::filesystem::path settingsDir("settings/");
      !std::filesystem::exists(settingsDir)) {
    std::filesystem::create_directory(settingsDir);
  }

  std::fstream settingsFile;
  settingsFile.open("settings/startup.bin",
                    std::ios_base::in | std::ios_base::binary);

  std::uint16_t fps{};
  bool vsync{};
  bool screen{};

  if (!settingsFile.is_open()) {
    std::uint16_t defaultFPS{60};
    bool defaultVsync{true};
    bool defaultScreen{true};

    fps = defaultFPS;
    vsync = defaultVsync;
    screen = defaultScreen;

    settingsFile.clear();
    settingsFile.open("settings/startup.bin", std::ios::out);

    settingsFile.write(reinterpret_cast<const char *>(&defaultFPS),
                       sizeof(defaultFPS));

    settingsFile.write(reinterpret_cast<const char *>(&defaultVsync),
                       sizeof(defaultVsync));

    settingsFile.write(reinterpret_cast<const char *>(&defaultScreen),
                       sizeof(defaultScreen));

    settingsFile.close();
  } else {

    std::array<char, 2> fpsBuffer;
    settingsFile.read(fpsBuffer.data(), fpsBuffer.size());

    fps = std::bit_cast<std::uint16_t>(fpsBuffer);

    char vsyncBuffer;
    settingsFile.read(&vsyncBuffer, sizeof(vsyncBuffer));
    vsync = vsyncBuffer;

    char screenBuffer;
    settingsFile.read(&screenBuffer, sizeof(screenBuffer));
    screen = screenBuffer;
  }

  if (!screen) {
    m_window.create(sf::VideoMode({1920, 1080}), "Band-Hero",
                    sf::State::Windowed);
  }

  m_window.setVerticalSyncEnabled(vsync);
  m_window.setFramerateLimit(fps);

  std::cout << "Window: " << screen << std::endl;
  std::cout << "Vsync: " << vsync << std::endl;
  std::cout << "FPS: " << fps << std::endl;

  Device::Init();
}

void Game::run() {
  ImGuiViewport *viewport = ImGui::GetMainViewport();

  ImGuiWindowFlags canvasFlags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoBackground;

  while (m_window.isOpen()) {
    sf::Time elapsed = m_clock.restart();

    handleEvents();

    ImGui::SFML::Update(m_window, elapsed);

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::Begin("GlobalGameCanvas", nullptr, canvasFlags);

    update(elapsed.asSeconds());

    ImGui::End();

    draw();
    m_stack.act();
  }

  ImGui::SFML::Shutdown();
}

void Game::handleEvents() {

  while (const auto event = m_window.pollEvent()) {

    ImGui::SFML::ProcessEvent(m_window, *event);

    if (event->is<sf::Event::Closed>()) {
      m_window.close();
    } else {
      m_stack.handleEvents(*event);
    }
  }
}

void Game::update(float dt) { m_stack.update(dt); }

void Game::draw() {

  m_window.clear(sf::Color(20, 20, 20));
  m_stack.draw(m_window);

  ImGui::SFML::Render(m_window);
  m_window.display();
}

} // namespace bh
