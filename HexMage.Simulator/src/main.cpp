#include <sim.hpp>

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
    auto manager = g.turn_manager();

    if (manager.is_turn_done()) {
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

// void gui() {
//  Game g;
//
//  while (!quit) {
//    auto input = process_input();
//
//    if (g.user_playing()) {
//      g.user_turn(input);
//    } else {
//      g.ai_turn();
//    }
//  }
//}

int main() {}