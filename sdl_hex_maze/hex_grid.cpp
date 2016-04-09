#include "hex_grid.hpp"

color color_for_type(HexType type) {
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
	float angle_deg = 60 * i + 30;
	return M_PI / 180 * angle_deg;
}

void push_vertex(std::vector<float>& vbo, float x, float y, color c) {
	vbo.push_back(x);
	vbo.push_back(y);
	push_color(vbo, c.r, c.g, c.b, c.a);
}

void hex_at(std::vector<float>& vertices, float x, float y, float r, color c) {
	float ri;
	int rot = 0; // 1;
	for (int i = rot; i < 7 + rot; i++) {
		push_vertex(vertices, x, y, c);

		ri = rad_for_hex(i - 1);
		c = c.mut(0.015f);
		push_vertex(vertices, x + r * cos(ri), y + r * sin(ri), c);

		ri = rad_for_hex(i);
		c = c.mut(0.015f);
		push_vertex(vertices, x + r * cos(ri), y + r * sin(ri), c);
	}
}


HexGrid::HexGrid(int m)
	: mat_dim_(m),
	data_(mat_dim_, mat_dim_),
	positions_(mat_dim_, mat_dim_),
	m(m) {
	// TODO - ugly
	std::fill(positions_.vs.begin(), positions_.vs.end(),
		std::make_pair<float, float>(INFINITY, INFINITY));
}

HexType& HexGrid::operator()(int col, int row) {
	return data_(col, row);
}

std::pair<float, float>& HexGrid::pos(int col, int row) {
	return positions_(col, row);
}

bool HexGrid::move_player(int col, int row) {
	if (fmax(std::abs(row), std::abs(col)) >= m || fmin(row, col) < 0)
		return false;
	if ((*this)(col, row) == HexType::Wall)
		return false;

	//if (player_set) {
	//	(*this)(player_col, player_row) = HexType::Empty;
	//}

	player_row = row;
	player_col = col;

	//(*this)(col, row) = HexType::Player;

	player_set = true;
	return true;
}

bool HexGrid::step_player(int dcol, int drow) {
	if (player_set) {
		return move_player(player_col + dcol, player_row + drow);
	}
	return false;
}

std::pair<int, int> HexGrid::highlight_near(float rel_x, float rel_y) {
	int closest_x = 2;
	int closest_y = 2;
	float min = INFINITY;

	for (size_t i = 0; i < positions_.m; i++) {
		for (size_t j = 0; j < positions_.n; j++) {
			//if ((*this)(i, j) == HexType::Player)
			//	(*this)(i, j) = HexType::Empty;

			auto pos = positions_(i, j);
			float d1 = pos.first - rel_x;
			float d2 = pos.second - rel_y;

			float distance = d1 * d1 + d2 * d2;
			if (distance < min) {
				closest_x = j;
				closest_y = i;
				min = distance;
			}
		}
	}

	return{ closest_y, closest_x };
	//(*this)(closest_y, closest_x) = HexType::Player;
}

void HexGrid::prebuild_VBO(float radius) {
	float start_x = -0.5f;
	float start_y = -0.5f;

	float width = cos(30 * M_PI / 180) * radius * 2;
	float height_offset = radius + sin(30 * M_PI / 180) * radius;

	vertices.reserve(1000);

	for (int row = 0; row < m; ++row) {
		for (int col = 0; col < m; ++col) {
			float draw_x = start_x;
			float draw_y = start_y;

			// axial q-change
			draw_x += col * width;
			// axial r-change
			draw_x += row * (width / 2);
			draw_y += row * height_offset;

			pos(col, row) = { draw_x, draw_y };

			color c = color_for_type((*this)(col, row));

			auto p = pos(col, row);
			hex_at(vertices, p.first, p.second, radius, c);
			c = c.mut(0.004f);
		}
	}
}