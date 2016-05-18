#include <sim.hpp>

sim::Team::Team(int number)
  : number(number)
{
  using namespace std;
  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<float> dis(0.0f, 1.0f);

  color = { dis(gen), dis(gen), dis(gen) };
}
