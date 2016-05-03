#include <iostream>
#include <queue>
#define _USE_MATH_DEFINES
#include <math.h>

#include <stopwatch.hpp>
#include <gl_utils.hpp>
#include <model.hpp>
#include <boost/optional.hpp>

namespace model {
	Arena::Arena(std::size_t size): size(size), hexes(size), positions(size), paths(size) {
		gl::Vertex::setup_attributes();
		shader.set("projection", glm::mat4(1.0f));
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
				
				p.source = boost::none;
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

			if (iterations > 10000 || queue.size() > 1000) {
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

				Color c = color_for_type((*this)({ col, row }));

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

}
