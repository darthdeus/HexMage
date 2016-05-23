#include <numeric>
#include <iterator>
#include <queue>
#include <vector>

#include <sim.hpp>

namespace sim
{
	Ability::Ability(int d_hp, int d_ap, int cost): d_hp(d_hp), d_ap(d_ap), cost(cost) {}

	Target::Target(Mob& mob) : mob_(mob) {}

	Map::Map(std::size_t size): size_(size), hexes_(size, size) {}

	Matrix<glm::vec2> Map::hexes() { return hexes_; }

	Index<Mob> MobManager::add_mob(Mob mob) {
		mobs_.push_back(std::move(mob));
	}

	Index<Team> MobManager::add_team() {
		teams_.emplace_back(teams_.size() + 1);
		return Index<Team>{teams_, teams_.size() - 1};
	}

	std::vector<Mob>& MobManager::mobs() {
		return mobs_;
	}

	std::vector<Team>& MobManager::teams() {
		return teams_;
	}

	Pathfinder::Pathfinder(std::size_t size): paths_(size, size) {}

	Matrix<Path>& Pathfinder::paths() { return paths_; }

	Path Pathfinder::path_to(Target target) {
		
	}

	void Pathfinder::move_as_far_as_possible(Mob& mob, Path& path) {
		
	}

	void Pathfinder::pathfind_from(glm::vec2 source) {
		
	}

	std::size_t Pathfinder::distance(glm::vec2 t1, glm::vec2 t2) {
		
	}

	void Pathfinder::update(glm::vec2 start, Map& map, MobManager& mob_manager) {
		std::queue<glm::vec2> queue;
		queue.push(start);

		std::vector<glm::vec2> diffs = {
			{ -1, 0 },
			{ 1, 0 },
			{ 0, -1 },
			{ 0, 1 },
			{ 1, -1 },
			{ -1, 1 }
		};

		for (std::size_t i = 0; i < paths_.m; ++i) {
			for (std::size_t j = 0; j < paths_.n; ++j) {
				auto&& p = paths_(i, j);
				
				p.source = boost::none;
				p.distance = std::numeric_limits<int>::max();

			}
		}
	}

	TurnManager::TurnManager(MobManager& mob_manager): mob_manager_(mob_manager) {}

	bool TurnManager::is_turn_done() const {
		return current_ < turn_order_.size();
	}

	void TurnManager::start_next_turn() {
		turn_order_.clear();
		for (auto&& mob : mob_manager_.mobs()) {
			// TODO - per-turn effects should be applied here
			mob.ap = mob.max_ap;

			if (mob.hp > 0) {
				turn_order_.push_back(&mob);
			}
		}

		auto ap_compare = [](auto m1, auto m2) -> bool {
			return m1->ap < m2->ap;
		};
		std::sort(turn_order_.begin(), turn_order_.end(), ap_compare);
	}

	Mob& TurnManager::current_mob() const {
		assert(current_ < turn_order_.size());
		return *turn_order_[current_];
	}

	bool TurnManager::move_next() {
		if (!is_turn_done()) {
			++current_;
		}

		return is_turn_done();
	}

	void UsableAbility::use() {}

	Game::Game(std::size_t size) : map_(size),
	                            mob_manager_{}, pathfinder_(size),
	                            turn_manager_(mob_manager_), size_(size) {}

	MobManager& Game::players() {
		return mob_manager_;
	}

	Pathfinder& Game::pathfinder() {
		return pathfinder_;
	}

	TurnManager& Game::turn_manager() {
		return turn_manager_;
	}

	Index<Mob> Game::add_mob(Mob mob) {
		return mob_manager_.add_mob(mob);
	}

	bool Game::is_finished() {
		auto mob_alive = [](auto mob) {
			return mob.hp > 0;
		};

		// TODO - introduce a faster check than manually iterate everything
		for (auto&& team : mob_manager_.teams()) {
			auto&& mobs = team.mobs();
			bool none_alive = std::none_of(mobs.begin(), mobs.end(), mob_alive);

			if (none_alive)
				return true;
		}

		return false;
	}

	std::size_t Game::size() const {
		return size_;
	}

	Index<Team> Game::add_team() {
		return mob_manager_.add_team();
	}

	std::vector<Ability> Game::usable_abilities(Mob& mob) const {
		std::vector<Ability> result;
		for (auto&& ability : mob.abilities) {
			if (ability.cost <= mob.ap) {
				result.push_back(ability);
			}
		}

		return result;
	}

	std::vector<UsableAbility> Game::usable_abilities(Mob&, Target, MobManager&, Pathfinder&) { }

	std::vector<Target> Game::possible_targets(Mob& mob, MobManager& mob_manager, Pathfinder& pathfinder) {
		auto&& abilities = mob.abilities;

		auto f_max = [](auto acc, auto&& ability) { return std::max(acc, ability.range)};
		auto max = std::accumulate(abilities.begin(), abilities.end(), 0, f_max);

		std::vector<Target> result;

		for (auto&& enemy : mob_manager.mobs()) {
			if (pathfinder.distance(mob.c, enemy.c) <= max) {
				result.emplace_back(enemy.c);
			}
		}

		return result;
	}


	Mob::Mob(int max_hp, int max_ap, abilities_t abilities, Index<Team> team)
		: max_hp(max_hp),
		  max_ap(max_ap),
		  abilities(std::move(abilities)),
		  team(std::move(team)) {}

	Team::Team(int number) : number(number) {
		using namespace std;
		random_device rd;
		mt19937 gen(rd());
		uniform_real_distribution<float> dis(0.0f, 1.0f);

		color_ = {dis(gen), dis(gen), dis(gen)};
	}

	void Team::add_mob(Mob& mob) {
		mobs_.push_back(&mob);
	}

	int Team::id() const {
		return number;
	}

	glm::vec3& Team::color() {
		return color_;
	}

	std::vector<Mob*> Team::mobs() {
		return mobs_;
	}
}
