#pragma once

// TODO - fuj, cmath
#define _USE_MATH_DEFINES
#include <cmath>

#include "gl_utils.hpp"

enum class HexType
{
	Empty = 0,
	Player,
	Wall
};

color color_for_type(HexType type);
float rad_for_hex(int i);
void push_vertex(std::vector<float>& vbo, float x, float y, color c);
void hex_at(std::vector<float>& vertices, float x, float y, float r, color c);

class HexGrid
{
	std::size_t mat_dim_;
	matrix<HexType> data_;
	matrix<std::pair<float, float>> positions_;

public:
	int m;
	int player_row = 0, player_col = 0;
	bool player_set = false;
	std::vector<float> vertices;

	HexGrid(int m);
	HexGrid(const HexGrid&) = delete;

	HexType& operator()(int col, int row);

	std::pair<float, float>& pos(int col, int row);
	bool move_player(int col, int row);
	bool step_player(int dcol, int drow);
	std::pair<int, int> highlight_near(float rel_x, float rel_y);
	inline std::pair<int, int> highlight_near(Coord c) { return highlight_near(c.x, c.y); }
	void prebuild_VBO(float radius);
};

