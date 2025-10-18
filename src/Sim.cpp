#include <memory>
#include <vector>
#include <unordered_set>
#include <limits>
#include <utility>

#include "Sim.hpp"
#include "Utils.hpp"

bool Simulator::run_sim(const double speed) {
  RUNTIME_EXCEPTION(car != nullptr, "Car is null");

  // Write your implementation here
  /*
  Idea:
  - simulate the race by updating at each route point
  - track charging along the route by looking up irradiance values every 30 seconds while on the road
  - note: it takes energy to start moving whenever we stop
  - i am not dealing with australian daylight savings time even though i know that south australia uses it and northern territory does not.
  */
  std::vector<Coord> route_points = route.get_route_points();
  int day = 0;
  double energy = 5.2; // in kWh
  double irradiance = 0;
  Time time = day_one_start_time;
  Time cur_time = day_one_start_time;
  Time temp = day_one_start_time;

  car->set_velocity(speed);

  for (int i = 0; i < route_points.size() - 1; i++) {
      // Handling charging and passive energy loss when at a control stop
      if (control_stops.find(i) != control_stops.end()) {
          // since the first stop is nonzero, and since our while loop below ends at the next stop, we should have the correct irradiance. we account for the energy required to get the car moving here as well.
          energy += car->charge(irradiance, control_stop_charge_time) - car->stop_passive_loss() - car->energy_to_move();
          time = time + control_stop_charge_time;
          continue;
      }

      if (i == 0) {
          energy -= car->energy_to_move();
      }

      car->update_attributes(get_distance(route.get_route_points()[i], route.get_route_points()[i + 1]), get_height_difference(route.get_route_points()[i], route.get_route_points()[i + 1]));
      time.update_time_seconds(car->get_time_to_dst());
      energy -= car->total_energy_loss();
      temp = cur_time;
      temp.update_time_seconds(CHARGING_STEP_SIZE); // temp is 30 seconds ahead always, so that our while loop actually works

      // loop to account for charge gotten back before car reaches next route point or before the day ends
      
      // handling overnight charging and splitting the time into two segments - first segment ends at the end of one day and the second segment starts the next day.
      // if it takes more than two days/two segments to get between two route points you're too slow anyways
      
      if (time > day_one_end_time && !day) { // if it's day 1 (day == 0)
          time = time + (day_start_time - day_one_end_time);
          energy = 5.2; // placeholder because i can't figure out the problem with my overnight charging code
          /*
          while (temp <= day_one_end_time) {
              irradiance = forecast_lut.get_value({ route_points[i].lat, route_points[i].lon }, cur_time.get_utc_time_point());
              energy = std::min(5.2, energy + car->charge(irradiance, CHARGING_STEP_SIZE));
              cur_time.update_time_seconds(CHARGING_STEP_SIZE);
              temp.update_time_seconds(CHARGING_STEP_SIZE);
          }
          if (energy <= 0) { return false; } // this isn't completely accurate because we took away the total energy loss between the route points all at once
          */
          energy -= car->energy_to_move(); // energy might just be capped at 5.2, which isn't necessarily true after the car starts moving again. this attempts to partially account for this
          cur_time = time; //also placeholder code
          /*
          while (temp <= time) {
              irradiance = forecast_lut.get_value({ route_points[i].lat, route_points[i].lon }, cur_time.get_utc_time_point());
              energy = std::min(5.2, energy + car->charge(irradiance, CHARGING_STEP_SIZE));
              cur_time.update_time_seconds(CHARGING_STEP_SIZE);
              temp.update_time_seconds(CHARGING_STEP_SIZE);
          }
          */
          day++;
      }
      else if (time > day_end_time) {
          time = time + (day_start_time - day_end_time);
          energy = 5.2; // placeholder because i can't figure out the problem with my overnight charging code
          /*
          while (temp <= day_end_time) {
              irradiance = forecast_lut.get_value({ route_points[i].lat, route_points[i].lon }, cur_time.get_utc_time_point());
              energy = std::min(5.2, energy + car->charge(irradiance, CHARGING_STEP_SIZE));
              cur_time.update_time_seconds(CHARGING_STEP_SIZE);
              temp.update_time_seconds(CHARGING_STEP_SIZE);
          }
          if (energy <= 0) { return false; } // this isn't completely accurate because we took away the total energy loss between the route points all at once
          */
          energy -= car->energy_to_move(); // energy might just be capped at 5.2, which isn't necessarily true after the car starts moving again. this attempts to partially account for this
          cur_time = time; // also placeholder code
          /*
          while (temp <= time) {
              irradiance = forecast_lut.get_value({ route_points[i].lat, route_points[i].lon }, cur_time.get_utc_time_point());
              energy = std::min(5.2, energy + car->charge(irradiance, CHARGING_STEP_SIZE));
              cur_time.update_time_seconds(CHARGING_STEP_SIZE);
              temp.update_time_seconds(CHARGING_STEP_SIZE);
          }
          */
          day++;
      }
      else {
          while (temp <= time) {
              irradiance = forecast_lut.get_value({ route_points[i].lat, route_points[i].lon }, cur_time.get_utc_time_point());
              energy = std::min(5.2, energy + car->charge(irradiance, CHARGING_STEP_SIZE));
              cur_time.update_time_seconds(CHARGING_STEP_SIZE);
              temp.update_time_seconds(CHARGING_STEP_SIZE);
          }
      }


      if (energy <= 0 || day >= 7) {
          return false;
      }

      if (i + 1 == route_points.size() - 1) {
          std::cout << time.get_local_readable_time() << std::endl;
      }
  }

  return true;
}

Simulator::Simulator(std::shared_ptr<Car> model, const Coord starting_coord,
                     const Time starting_time) : car(model), starting_coord(starting_coord),
                                                 curr_time(starting_time) {}
