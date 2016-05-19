#include <sim.hpp>

namespace sim {

Target::Target(Mob& mob) : mob_(mob) {}

Path Pathfinder::path_to(Target target) {}

void Pathfinder::move_as_far_as_possible(Mob& mob, Path& path) {}

bool TurnManager::is_turn_done() const { return false; }

void TurnManager::start_next_turn() {}

Mob& TurnManager::current_mob() const {}

void UsableAbility::use() {}

Players& Game::players() { return players_; }
Pathfinder& Game::pathfinder() { return pathfinder_; }
TurnManager& Game::turn_manager() { return turn_manager_; }
Mob& Game::add_mob(Mob mob) {}

bool Game::is_finished() const {
}

Team::Team(int number) : number(number) {
  using namespace std;
  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<float> dis(0.0f, 1.0f);

  color = {dis(gen), dis(gen), dis(gen)};
}
}
