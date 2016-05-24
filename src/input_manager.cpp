#include <glad/glad.h>
#include <SDL/SDL.h>
#include <input_manager.hpp>
#include <imgui_impl_sdl.h>
#include <game.hpp>

using namespace sim;
using namespace glm;

void InputManager::mousemove(glm::vec2 pos, sim::Mob& player) {
  highlight_hex = game::hex_at_mouse(camera_.projection(), geom_, pos);

  mouse_hex = highlight_hex;

  auto&& pathfinder = game_.pathfinder();
  auto&& mob_manager = game_.mob_manager();
  highlight_path = pathfinder.path_to(highlight_hex);
  // highlight_path = build_highlight_path(player);
}

void InputManager::left_click(glm::vec2 pos, sim::Mob& current_mob) {
  auto click_hex = game::hex_at_mouse(camera_.projection(), geom_, pos);

  // auto&& player = current_mob.team->player();
  //
  // if (player.is_ai()) {
  // 	fmt::print("Forcing AI to take a turn\n");
  // 	player.any_action(game_, current_mob);
  // } else {
  // player.action_to(click_hex, game_, current_mob);

  // highlight_hex = current_mob.c;
  // highlight_path.clear();

  // }

  auto&& pathfinder = game_.pathfinder();
  pathfinder.pathfind_from(current_mob.c, game_.map(), game_.mob_manager());
  geom_.regenerate_geometry(current_mob.ap);
}

void InputManager::right_click(glm::vec2 pos, Mob& player) {
  auto click_hex = game::hex_at_mouse(camera_.projection(), geom_, pos);
  auto&& map = game_.map();

  if (map(click_hex) == HexType::Empty) {
    map(click_hex) = HexType::Wall;
  } else {
    map(click_hex) = HexType::Empty;
  }

  auto&& pathfinder = game_.pathfinder();
  auto&& current_mob = game_.turn_manager().current_mob();
  pathfinder.pathfind_from(current_mob.c, game_.map(), game_.mob_manager());
  geom_.regenerate_geometry();
}

// std::vector<sim::Coord> InputManager::build_highlight_path(const sim::Mob& player) {
//   using namespace sim;
//   std::vector<sim::Coord> path;
//
//   if (arena_(highlight_hex) != HexType::Wall) {
//     while (highlight_hex != player.c) {
//       if (arena_(highlight_hex) == HexType::Wall) {
//         path.clear();
//         break;
//       }
//
//       if (auto source = arena_.paths(highlight_hex).source) {
//         highlight_hex = *source;
//         if (arena_(highlight_hex) == HexType::Empty) {
//           path.push_back(highlight_hex);
//         }
//       } else {
//         path.clear();
//         break;
//       }
//     }
//   }
//
//   return path;
// }

bool InputManager::handle_events() {
  using namespace sim;

  auto&& turn_manager = game_.turn_manager();

  while (SDL_PollEvent(&event)) {
    ImGui_ImplSdlGL3_ProcessEvent(&event);

    // TODO - rewrite this

    if (!turn_manager.is_turn_done()) {
      auto&& current_mob = turn_manager.current_mob();

      switch (event.type) {
        case SDL_MOUSEMOTION:
          mousemove({event.motion.x, event.motion.y}, current_mob);
          break;

        case SDL_MOUSEBUTTONDOWN:
          glm::vec2 pos{event.motion.x, event.motion.y};
          switch (event.button.button) {
            case SDL_BUTTON_LEFT:
              left_click(pos, current_mob);
              break;
            case SDL_BUTTON_RIGHT:
              right_click(pos, current_mob);
              break;
          }
      }

      if (event.type == SDL_QUIT ||
          (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE))
        return false;
    }

    if (event.type == SDL_KEYDOWN) camera_.keydown(event.key.keysym.sym);
    if (event.type == SDL_KEYUP)
      if (event.key.keysym.sym == SDLK_SPACE) {
        auto&& turn_manager = game_.turn_manager();

        if (turn_manager.move_next()) {
          fmt::printf("DEBUG - moving to next player\n");
          game_.refresh();
          geom_.regenerate_geometry(turn_manager.current_mob().ap);
        } else {
          fmt::print("DEBUG - starting new turn\n");
          turn_manager.start_next_turn();
        }

        // // TODO - dijkstra for current player
        // auto* next_player = turn_manager_.current_turn.next();
        // if (next_player) {
        //   arena_.dijkstra(next_player->c, game_);
        //   arena_.regenerate_geometry(next_player->ap);
        // }
        //
        // if (turn_manager_.current_turn.is_done()) {
        //   // TODO - use proper logging
        //   fmt::print("DEBUG - starting new turn\n");
        //   turn_manager_.current_turn = game_.start_turn();
        // }
      } else {
        camera_.keyup(event.key.keysym.sym);
      }
    if (event.type == SDL_MOUSEWHEEL) camera_.scroll(event.wheel.y);
  }

  return true;
}
