#include <iostream>
#include <queue>
#define _USE_MATH_DEFINES
#include <math.h>

#include <stopwatch.hpp>
#include <gl_utils.hpp>
#include <model.hpp>
#include <boost/optional.hpp>

namespace model {
	int hex_distance(Coord a, Coord b) {
		using std::abs;
		return (abs(a.x - b.x)
			+ abs(a.x + a.y - b.x - b.y)
			+ abs(a.y - b.y)) / 2;
	}

	Cube::Cube(const Coord& axial) : x(axial.x), y(-axial.x - axial.y), z(axial.y) {}
	Cube::operator Coord() const { return{ *this }; }
	Cube Cube::abs() const { return{ std::abs(x), std::abs(y), std::abs(z) }; }
	int Cube::min() const { return std::min(x, std::min(y, z)); }
	int Cube::max() const { return std::max(x, std::max(y, z)); }

	Coord::Coord(const Cube& cube) : x(cube.x), y(cube.z) {}
	Coord::operator Cube() const { return{ *this }; }
	Coord Coord::abs() const { return{ std::abs(x), std::abs(y) }; }
	int Coord::min() const { return std::min(x, y); }
	int Coord::max() const { return std::max(x, y); }
	int Coord::distance() const { return Cube(*this).abs().max(); }

	Coord operator+(const Coord& lhs, const Coord& rhs) { return{ lhs.x + rhs.x, lhs.y + rhs.y }; }
	Coord operator-(const Coord& lhs, const Coord& rhs) { return{ lhs.x - rhs.x, lhs.y - rhs.y }; }
	bool operator==(const Coord& lhs, const Coord& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }

	Position operator+(const Position& lhs, const Position& rhs) { return{ lhs.x + rhs.x, lhs.y + rhs.y }; }
	Position operator-(const Position& lhs, const Position& rhs) { return{ lhs.x - rhs.x, lhs.y - rhs.y }; }
	bool operator==(const Position& lhs, const Position& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }

	std::ostream& operator<<(std::ostream& os, const Position& p) {
		return os << "(" << p.x << "," << p.y << ")";
	}

	float Position::distance() const { return x * x + y * y; }

	Position Position::operator-() const { return{ -x, -y }; }
	Position::operator glm::vec2() const { return{ x, y }; }

	Position Position::abs() const { return{ std::abs(x), std::abs(y) }; }
	float Position::min() const { return std::min(x, y); }
	float Position::max() const { return std::max(x, y); }

	Position& Position::operator+=(const Position& p) { x += p.x; y += p.y; return *this; }
	Position& Position::operator-=(const Position& p) { x -= p.x; y -= p.y; return *this; }

	std::ostream& operator<<(std::ostream& os, const Coord& c) {
		return os << "(" << c.x << "," << c.y << ")";
	}

	Position mouse2gl(int x, int y) {
		float rel_x = static_cast<float>(x) / SCREEN_WIDTH;
		float rel_y = static_cast<float>(y) / SCREEN_HEIGHT;

		rel_y = 2 * (1 - rel_y) - 1;
		rel_x = 2 * rel_x - 1;

		return{ rel_x, rel_y };
	}

	float rnd(float max) {
		return (static_cast<float>(rand()) / RAND_MAX) * max;
	}

	float rnd() {
		return rnd(1.0f);
	}

	float clamp(float x) {
		if (x < 0)
			return 0;
		if (x > 1)
			return 1;
		return x;
	}

	Arena::Arena(std::size_t size) : size(size), hexes(size), positions(size), paths(size) {
		gl::Vertex::setup_attributes();
		shader.set("projection", glm::mat4(1.0f));
	}

	bool Arena::is_valid_coord(const Coord& c) const {
		return static_cast<std::size_t>(c.abs().max()) < size && c.min() >= 0;
	}

	HexType& Arena::operator()(Coord c) { return hexes(c); }
	Position& Arena::pos(Coord c) { return positions(c); }

	Coord Arena::hex_near(Position rel_pos) {
		Coord closest;
		float min = INFINITY;

		for (std::size_t i = 0; i < positions.m; ++i) {
			for (std::size_t j = 0; j < positions.n;++j) {
				auto pos = positions(i, j);

				float distance = (pos - rel_pos).distance();

				if (distance < min) {
					closest = { static_cast<int>(j), static_cast<int>(i) };
					min = distance;
				}
			}
		}

		return closest;
	}

	void Arena::dijkstra(Coord start, PlayerInfo& info) {
		// TODO - proper log levels
		fmt::printf("DEBUG Dijkstra started at %i,%i\n", start.x, start.y);
		Stopwatch s;

		std::queue<Coord> queue;

		queue.push(start);

		std::vector<Coord> diffs = {
			{ -1, 0 },
			{ 1, 0 },
			{ 0, -1 },
			{ 0, 1 },
			{ 1, -1 },
			{ -1, 1 }
		};

		int iterations = 0;

		for (int i = 0; i < paths.m; ++i) {
			for (int j = 0; j < paths.n; ++j) {
				auto& p = paths(i, j);
				auto& h = hexes(i, j);

				p.source = boost::none;
				p.distance = std::numeric_limits<int>::max();

				if (h == HexType::Wall || info.mob_at({j, i})) {
					p.state = VertexState::Closed;
				} else {
					p.state = VertexState::Unvisited;
				}
			}
		}

		paths(start).distance = 0;
		paths(start).state = VertexState::Open;

		while (!queue.empty()) {
			Coord current = queue.front();
			queue.pop();

			if (iterations > 10000 || queue.size() > 1000) {
				std::cout << "dijkstra got stuck\titerations: " << iterations << ", queue: " << queue.size() << std::endl;
				return;
			}

			iterations++;

			Path& p = paths(current);

			if (p.state == VertexState::Closed) continue;

			p.state = VertexState::Closed;

			for (auto diff : diffs) {
				auto neighbour = current + diff;
				if (is_valid_coord(neighbour)) {
					Path& n = paths(neighbour);

					if (n.state != VertexState::Closed) {
						if (n.distance > p.distance + 1) {
							n.distance = p.distance + 1;
							assert(n.distance > 0);

							n.source = current;
						}

						n.state = VertexState::Open;
						queue.push(neighbour);
					}
				}
			}
		}
	}

	void Arena::regenerate_geometry(boost::optional<int> current_ap) {
		float start_x = -0.5f;
		float start_y = -0.5f;

		float width = static_cast<float>(cos(30 * M_PI / 180) * radius * 2);
		float height_offset = static_cast<float>(radius + sin(30 * M_PI / 180) * radius);

		b.clear();

		int isize = static_cast<int>(size);
		for (int row = 0; row < isize; ++row) {
			for (int col = 0; col < isize; ++col) {
				float draw_x = start_x;
				float draw_y = start_y;

				// axial q-change
				draw_x += col * width;
				// axial r-change
				draw_x += row * (width / 2);
				draw_y += row * height_offset;

				pos({ col, row }) = { draw_x, draw_y };

				auto type = (*this)({ col, row });
				Color c = color_for_type(type);
				auto path = paths({ col, row });

				if (path.distance < 0) {
					if (path.source) {
						fmt::printf("Distance negative %i at %i,%i, source %i,%i\n", path.distance, row, col, path.source->x, path.source->y);
					} else {
						
						fmt::printf("Distance negative %i at %i,%i, no source\n", path.distance, row, col);
					}
				}

				if (current_ap) {
					if (type == HexType::Empty && path.distance > 0) {
						// distance = 1 -> 0.3
						// distance = ap -> 0
						float change = (*current_ap + 1 - path.distance) * 0.06f;
						if (change > 0) {
							c = c.mut(change);
						}
					}
				}

				auto pos = this->pos({ col, row });
				b.push_hex({ pos.x, pos.y, 0 }, c, radius);
				//hex_at(vertices, pos, radius, c);
				c = c.mut(0.004f);
			}
		}

	}

	void Arena::draw_vertices() {
		shader.use();
		vao.bind();
		vbo.bind();
		b.draw_arrays();

		//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		//glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()) / 6);
	}

	void Arena::set_projection(const glm::mat4& projection) {
		shader.set("projection", projection);
	}

	void Arena::paint_hex(Position pos, float radius, Color color) {
		vao.bind();
		vbo.bind();
		shader.use();
		gl::Batch b;
		b.push_hex(pos, color, radius);
		b.draw_arrays();
	}

	void Arena::paint_healthbar(glm::vec2 pos, float hp, float ap)
	{
		using namespace glm;
		using namespace std;

		vao.bind();
		vbo.bind();
		shader.use();

		gl::Batch b;
		float width = Arena::radius / 5 * 2;
		float height = Arena::radius * 0.7f * 2;

		float hp_max = height * hp;
		float ap_max = height * ap;

		b.push_quad_bot_left(
		{ pos.x - width, pos.y - height / 2 },
			width, height, 0, { 0, 0.5, 0, 1 }
		);
		b.push_quad_bot_left(
		{ pos.x - width, pos.y - height / 2 },
			width, hp_max, 0, { 0, 1, 0, 1 }
		);

		b.push_quad_bot_left(
		{ pos.x, pos.y - height / 2 },
			width, height, 0, { 0.5, 0.5, 0, 1 }
		);
		b.push_quad_bot_left(
		{ pos.x, pos.y - height / 2 },
			width, ap_max, 0, { 1, 1, 0, 1 }
		);

		b.draw_arrays();
	}

	void Arena::paint_mob(TurnManager& turn_manager, PlayerInfo& info, const Mob& mob)
	{
		auto p = pos(mob.c);
		auto c = mob.team->color;
		if (&mob == turn_manager.current_mob()) {
			c += glm::vec3(0.3f);
		}
		auto col = Color{ c.r, c.g, c.b };
		paint_hex(p, radius, col);
		paint_healthbar(p, (float)mob.hp / mob.max_hp, (float)mob.ap / mob.max_ap);
	}

	Mob::Mob(int max_hp, int max_ap, abilities_t abilities, Index<Team> team) : max_hp(max_hp),
		max_ap(max_ap),
		hp(max_hp),
		ap(max_ap),
		abilities(abilities),
		team(team)
	{
		assert(abilities.size() == ABILITY_COUNT);
	}

	bool Mob::use_ability(int index, Target target)
	{
		assert(index <= abilities.size());

		auto& ability = abilities[index];
		return ap >= ability.cost;
	}

	void Mob::move(GameInstance& game, Coord d)
	{
		auto& arena = game.arena;
		auto new_coord = c + d;
		if (arena.is_valid_coord(new_coord) && !game.info.mob_at(new_coord)) {
			int cost = arena.paths(c).distance + 1;

			if (cost <= ap) {
				fmt::printf("Moving for %d AP\n", cost);
				c = c + d;
				ap -= cost; // TODO - better calculation
			}
		}
	}

	bool Mob::can_use_ability_at(Target t, PlayerInfo& info, Arena& arena, Ability ability)
	{
		bool within_range = ability.range >= arena.paths(t.c).distance;
		int distance = hex_distance(t.c, c);
		within_range = distance <= ability.range;

		return ability.cost <= ap && within_range;
	}

	Mob::abilities_t Mob::usable_abilities(Target t, PlayerInfo& info, Arena& arena)
	{
		abilities_t res;

		for (auto&& ability : abilities) {
			if (can_use_ability_at(t, info, arena, ability)) {
				res.push_back(ability);
			}
		}

		return res;
	}

	PlayerInfo::PlayerInfo(std::size_t size) : size(size) {}

	Mob& PlayerInfo::add_mob(Mob mob)
	{
		mobs.push_back(mob);
		return mobs.back();
	}

	Mob* PlayerInfo::mob_at(Coord c)
	{
		for (auto& m : mobs) {
			if (m.c == c) {
				return &m;
			}
		}

		return nullptr;
	}

	Index<Team> PlayerInfo::register_team(Player& player) {
		int id = static_cast<int>(teams.size());
		teams.emplace_back(id, player);
		return Index<Team>(teams, id);
	}

	Team& PlayerInfo::team_id(int id) {
		assert(id < teams.size());
		return teams[id];
	}

	boost::optional<Target> PlayerInfo::can_attack(Mob& player, Coord c)
	{
		if (Mob* mob = mob_at(c)) {
			if (player.team != mob->team) {
				return Target(c, *mob);
			} else {
				return boost::none;
			}
		} else {
			return boost::none;
		}
	}

	Turn GameInstance::start_turn()
	{
		for (auto&& mob : info.mobs) {
			mob.ap = std::min(mob.max_ap, mob.ap + mob.max_ap);
		}

		return Turn(info.mobs);
	}

	void TurnManager::update_arena(Arena& arena)
	{
		assert(!current_turn.is_done());
		auto& player = **current_turn.current_;

		// TODO - update this
		arena.dijkstra(player.c, info_);
		arena.regenerate_geometry();
	}

	Mob* TurnManager::current_mob() const
	{
		if (current_turn.is_done())
			return nullptr;
		else
			return *current_turn.current_;
	}

	void UserPlayer::action_to(Coord click_hex, GameInstance& game, Mob& current_mob)
	{
		auto path = game.arena.paths(click_hex);

		if (auto target = game.info.can_attack(current_mob, click_hex)) {
			auto abilities = current_mob.usable_abilities(*target, game.info, game.arena);

			if (abilities.size() > 0) {
				auto ability = abilities.back();
				fmt::print("Using ability {}\n", ability);

				current_mob.ap -= ability.cost;
				target->mob.hp = std::max(0, target->mob.hp - ability.d_hp);
			}
		}
		else {
			if (path.distance <= current_mob.ap) {
				current_mob.ap -= path.distance;
				current_mob.c = click_hex;
			}
		}
	}

	void UserPlayer::any_action(GameInstance& game, Mob& mob)
	{
		fmt::printf("TODO - what should this actually do?");
	}

	void AIPlayer::action_to(Coord c, GameInstance& game, Mob& mob)
	{
		// TODO - basic AI		
	}

	void AIPlayer::any_action(GameInstance& game, Mob& mob)
	{
		std::vector<Mob*> enemies;
		for (auto&& enemy : game.info.mobs) {
			if (enemy.team != mob.team && enemy.hp > 0) {
				enemies.push_back(&enemy);
			}
		}

		if (enemies.size() > 0) {
			auto distance_f = [&mob](Mob* a, Mob* b) {
				return hex_distance(mob.c, a->c) < hex_distance(mob.c, b->c);
			};

			std::sort(enemies.begin(), enemies.end(), distance_f);

			auto&& enemy = *enemies[0];
			auto c = enemy.c;

			auto abilities = mob.usable_abilities(
				Target(c, enemy),
				game.info,
				game.arena);

			if (abilities.empty()) {
				fmt::print("DEBUG - no abilities available, moving instead\n");
				// no ability is in rage, we have to move
				auto dis = (c - mob.c);
				auto dx = dis.abs().x;
				if (!dx) dx = 1;

				auto dy = dis.abs().y;
				if (!dy) dy = 1;

				// TODO - the offset is wrong when going bottom-left
				Coord off{ dis.x / dx, dis.y / dy };

				mob.move(game, off);

			} else {
				// TODO - use a random ability for now
				UserPlayer{}.action_to(c, game, mob);
			}

		} else {
			// TODO - logging
			fmt::print("INFO - All enemies are dead\n");
		}
		
	}

	Turn::Turn(std::vector<Mob>& mobs)
	{
		for (auto&& mob : mobs) {
			mobs_.push_back(&mob);
		}

		std::sort(mobs_.begin(), mobs_.end(),
			[](auto x, auto y) { return x->ap < y->ap; });

		current_ = mobs_.begin();
	}

	Mob* Turn::next()
	{
		if (current_ == mobs_.end()) return nullptr;

		while (true) {
			++current_;
			if (current_ == mobs_.end()) {
				return nullptr;
			}

			if ((**current_).hp > 0) return *current_;
		}
	}

	Color color_for_type(HexType type) {
		switch (type) {
		case HexType::Empty:
			return{ 0.4f, 0.2f, 0.4f };
		case HexType::Wall:
			return{ 0.1f, 0.03f, 0.1f };
		case HexType::Player:
			return{ 0.7f, 0.4f, 0.7f };
		default:
			throw "invalid hex type";
		}
	}

	float rad_for_hex(int i) {
		float angle_deg = static_cast<float>(60 * i + 30);
		return static_cast<float>(M_PI) / 180 * angle_deg;
	}
}
