#include "core/Game.hpp"
#include "core/Scale.hpp"

int main() {

  bh::Game game(bh::OriginalWindowSize, "Band Hero");
  game.run();

  return 0;
}
