#ifndef MOB_HPP
#define MOB_HPP

#include <assert.h>
#include <vector>
#include <numeric>
#include <random>
#include <iostream>

constexpr int ABILITY_COUNT = 6;

template <typename T>
struct Matrix {
  std::size_t m;
  std::size_t n;
  std::vector<T> vs;

  Matrix(std::size_t m, std::size_t n) : m(m), n(n), vs(m * n) {}

  T& operator()(std::size_t i, std::size_t j) { return vs[n * i + j]; }

 private:
}; /* column-major/opengl: vs[i + m * j], row-major/c++: vs[n * i + j] */

class Ability {
 public:
  int d_hp;
  int d_ap;
  int cost;

  Ability(int d_hp, int d_ap, int cost) : d_hp(d_hp), d_ap(d_ap), cost(cost) {}
};

class Mob {
 public:
  using abilities_t = std::vector<Ability>;
  const int max_hp;
  const int max_ap;

  int hp;
  int ap;

  abilities_t abilities;

  // TODO - fuj
  int x = 0;
  int y = 0;

  Mob(int max_hp, int max_ap, abilities_t abilities)
      : max_hp(max_hp),
        max_ap(max_ap),
        hp(max_hp),
        ap(max_ap),
        abilities(abilities) {
    assert(abilities.size() == ABILITY_COUNT);
  }
};

enum class HexType { Empty = 0, Wall };

class Hex {
 public:
  HexType type;
};

class Arena {
 public:
  std::size_t size;

  Arena(std::size_t size) : size(size) {}
};

class PlayerInfo {
 public:
  std::vector<Mob> mobs;
  std::size_t size;

  PlayerInfo(std::size_t size) : size(size) {}

  void add_mob(Mob mob) { mobs.push_back(mob); }
  Mob& mob_at(int x, int y) {
    for (auto& m : mobs) {
      if (m.x == x && m.y == y) {
        return m;
      }
    }

    std::cerr << "Mob not found at " << x << "," << y << std::endl;
    throw "Mob not found";
  }
};

class GameInstance {
 public:
  Arena& arena;
  PlayerInfo& info;

  GameInstance(Arena& arena, PlayerInfo& info) : arena(arena), info(info) {}

  void start_turn() {}
};

class DummySimulation {
  void run() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(-10, 10);
    std::uniform_int_distribution<int> cost_dis(3, 7);

    Mob::abilities_t abilities;
    for (int i = 0; i < ABILITY_COUNT; ++i) {
      abilities.emplace_back(dis(gen), dis(gen), cost_dis(gen));
    }

    std::size_t map_size = 3;

    Arena arena(map_size);
    PlayerInfo info(map_size);

    GameInstance game(arena, info);

    Mob mob(10, 10, abilities);
    info.add_mob(mob);

    constexpr int SIM_TIME = 10;


    Mob& player = info.mob_at(0, 0);

    for (int i = 0; i < SIM_TIME; ++i) {

    }
  }
};

#endif /* MOB_HPP */
