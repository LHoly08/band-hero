#include "core/Game.hpp"

int main() {

  bh::Game game({.x = 1280, .y = 720}, "Band Hero");
  game.run();

  return 0;
}
