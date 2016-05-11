#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#pragma once

#include <model.hpp>

namespace generator
{
	model::Mob random_mob(Index<model::Team> team, std::size_t size);
}

#endif