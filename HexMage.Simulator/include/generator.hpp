#ifndef HEXMAGE_SIM_GENERATOR_HPP
#define HEXMAGE_SIM_GENERATOR_HPP

#include <sim.hpp>

namespace gen {
  sim::Mob random_mob(Index<sim::Team> team, std::size_t size);
}


#endif /* HEXMAGE_SIM_GENERATOR_HPP */
