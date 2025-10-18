#pragma once

#include <memory>
#include <Utils.hpp>
#include <algorithm>

class Car {
 private:
	 const double AIR_DENSITY = 1.18; // in kg/m^3

  /* Define car parameters here according to the doc */
	int passive_electric_loss = 20; // in watts
	double rolling_resistance = 0.0026;
	double cda = 0.15; // coefficient of drag * frontal area?
	double array_efficiency = 0.252;
	int car_mass = 283; // in kg
	double battery_efficiency = 0.98;
	double motor_efficiency = 0.8;
	int array_area = 4; // in m^2
	double max_battery_capacity = 5.2; // in kWh

	// car attributes
	double distance;
	double velocity;
	double height_difference;
	double incline = 0;
	double time_to_dst;

 public:
  Car();

  // helper functions:
  void set_velocity(double velocity) {
	  this->velocity = velocity; // in mps
  }

  void update_attributes(double distance, double height_difference) {
	  this->distance = distance; // is this m or km???
	  this->height_difference = height_difference;
	  this->incline = atan(height_difference / distance);
	  this->time_to_dst = distance / velocity; // in seconds
  }

  double get_time_to_dst() { return time_to_dst; }

  // Define energy loss functions

  double aero_loss() {
	  // return (distance) * (0.5 * air_density * velocity^2 * cda); work = distance * drag force
	  return distance * (0.5 * AIR_DENSITY * velocity * velocity * cda) / JOULES_TO_KWH;
  }

  double rolling_loss() {
	  // assume constant incline
	  return distance * rolling_resistance * car_mass * GRAVITY_ACCELERATION * (cos(incline)) / JOULES_TO_KWH; // rolling resistance * weight = rolling resistance * normal force = rolling resistance * mass * cos angle of incline
  }
  double gravitational_loss() {
	  return car_mass * GRAVITY_ACCELERATION * distance * sin(incline) / JOULES_TO_KWH; 
  }
  double passive_energy_loss() {
	  // return passive_electric_loss * time, converted to kWh;
	  return (passive_electric_loss * time_to_dst) / JOULES_TO_KWH;
  }

  double stop_passive_loss() { // in kWh
	  return 1800 * passive_electric_loss / JOULES_TO_KWH;
  }

  double charge(double irradiance, double time) { // in kWh
	  return time * irradiance * array_area * array_efficiency / JOULES_TO_KWH;
  }

  double energy_to_move() {  // kinetic energy required to get the car moving
	  return (car_mass * velocity * velocity / 2) * (1 / battery_efficiency) * (1 / motor_efficiency) / JOULES_TO_KWH;
  }

  double total_energy_loss();
};
