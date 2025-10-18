#include "Car.hpp"
#include <algorithm>

Car::Car() {}

// Write implemenation for your energy model here
double Car::total_energy_loss() {
	return aero_loss() + rolling_loss() + gravitational_loss() + passive_energy_loss();
}
