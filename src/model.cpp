#include <iostream>
#include <queue>
#define _USE_MATH_DEFINES
#include <math.h>

#include <stopwatch.hpp>
#include <gl_utils.hpp>
#include <model.hpp>

namespace model {
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

	void Arena::dijkstra(Coord start) {
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
				

				p.distance = std::numeric_limits<int>::max();
				if (h == HexType::Wall) {
					p.state = VertexState::Closed;
				} else {
					p.state = VertexState::Unvisited;
				}
			}
		}

		while (!queue.empty()) {
			Coord current = queue.front();
			queue.pop();

			if (iterations > 1000000 || queue.size() > 100000) {
				std::cout << "got stuck heh\titerations: " << iterations << ", queue: " << queue.size() << std::endl;
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
							n.source = current;
						}

						n.state = VertexState::Open;
						queue.push(neighbour);
					}
				}
			}
		}
	}

	void Arena::regenerate_geometry() {
		float start_x = -0.5f;
		float start_y = -0.5f;

		float width = static_cast<float>(cos(30 * M_PI / 180) * radius * 2);
		float height_offset = static_cast<float>(radius + sin(30 * M_PI / 180) * radius);
		
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

				Color c = color_for_type((*this)({ col, row }));

				auto pos = this->pos({ col, row });
				hex_at(vertices, pos, radius, c);
				c = c.mut(0.004f);
			}
		}

	}

	void Arena::draw_vertices() {
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()) / 3);
	}

	Position mouse2gl(int x, int y) {
		// TODO - figure out a place to put this
		constexpr int SCREEN_WIDTH = 800;
		constexpr int SCREEN_HEIGHT = 600;

		float rel_x = static_cast<float>(x) / SCREEN_WIDTH;
		float rel_y = static_cast<float>(y) / SCREEN_HEIGHT;

		rel_y = 2 * (1 - rel_y) - 1;
		rel_x = 2 * rel_x - 1;

		return{ rel_x, rel_y };
	}
}
