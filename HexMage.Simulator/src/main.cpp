#include <sim.hpp>

namespace sim {

class Target {
 public:
  Target(Mob&);
};

class Path {};

class Map {};
class Players {};

class Pathfinder {
  Path path_to(Target target);
};

class TurnManager {};

class UsableAbility {
 public:
  void use();
};

class Game {
 public:
  Players& players();
  Pathfinder& pathfinder();
  TurnManager& turn_manager();

  Mob& add_mob(Mob mob);

  bool is_finished() const;

  // 1. jake schopnoasti muzu pouzit - sebe
  // 1b. jake schopnoasti muzu pouzit na policko - sebe, hrace, cesty
  std::vector<Ability> usable_abilities(Mob&);
  std::vector<UsableAbility> usable_abilities(Mob&, Target, Players&,
                                              Pathfinder&);

  // 2. na koho muzu utocit - sebe, hrace, cesty
  std::vector<Target> possible_targets(Mob&, Players&, Pathfinder&);

  //
  // 3. kdo je na tahu - tahovatko

  //
  // 4. kdo updatuje stav - hrace, sebe, mapu
  // 5. kdo resi cesty - hrace, sebe, mapu
};
}

namespace gen {
sim::Mob random_mob();
};

using namespace sim;

void testy() {
  Game g;

  auto m1 = g.add_mob(gen::random_mob());
  auto m2 = g.add_mob(gen::random_mob());

  auto abilities =
      g.usable_abilities(m1, Target(m2), g.players(), g.pathfinder());

  if (abilities.size() > 0) {
    auto ability = abilities[0];

    ability.use();

    assert(m2.hp < m2.max_hp);
  }
}

void mcts() {
  Game g;

  while (!g.is_finished()) {
    auto manager = g.turn_manager;

    if (manager.turn_is_done()) {
      manager.start_next_turn();
    } else {
      auto mob = manager.current_mob();

      auto targets = g.possible_targets(mob, g.players(), g.pathfinder());

      if (targets.size() > 0) {
        auto target = targets[0];

        auto abilities =
            g.usable_abilities(mob, target, g.players(), g.pathfinder());

        if (abilities.size() > 0) {
          abilities[0].use();
        } else {
          auto pathfinder = g.pathfinder();

          auto path = pathfinder.path_to(target);

          move_as_far_as_possible(mob, path);
        }
      }
    }
  }
}

void gui() {
  Game g;

  while (!quit) {
    auto input = process_input();

    if (g.user_playing()) {
      g.user_turn(input);
    } else {
      g.ai_turn();
    }
  }
}

int main() {}
