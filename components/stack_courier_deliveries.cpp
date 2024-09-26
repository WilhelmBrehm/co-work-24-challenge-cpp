#ifndef VRPPD_STACK_COURIER_DELIVERIES_CPP
#define VRPPD_STACK_COURIER_DELIVERIES_CPP

#include "vrppd_parameters.h"
#include "vrppd_solution.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

std::vector<std::vector<int>> catalan_combinations(int n) {
    std::vector<std::vector<int>> result;

    std::function<void(std::vector<int>&, int, int)> backtrack = [&](std::vector<int>& s, int open, int close) {
        if (s.size() == 2 * n) {
            result.push_back(s);
            return;
        }
        if (open < n) {
            s.push_back(0);
            backtrack(s, open + 1, close);
            s.pop_back();
        }
        if (close < open) {
            s.push_back(1);
            backtrack(s, open, close + 1);
            s.pop_back();
        }
    };

    std::vector<int> s;
    backtrack(s, 0, 0);

    for (auto& combination : result) {
        int open_bracket_count = 1;
        int closed_bracket_count = 1;
        for (auto& elem : combination) {
            if (elem == 0) {
                elem = open_bracket_count++;
            } else {
                elem = -closed_bracket_count++;
            }
        }
    }

    return result;
}


double delivery_time_of_rerouting(const VRPPDParameters& param, const VRPPDSolution& sol, int courier_index, const std::vector<int>& new_route) {
    double courier_attributed_delivery_time = 0;
    double current_time = 0;
    int current_location = param.courier_starting_location[courier_index - 1];
    double current_load = 0;
    
    for (int i = 0; i < 2 * sol.delivery_count_assigned_to_courier[courier_index - 1]; ++i) {
        if (new_route[i] > 0) {
            current_load += param.delivery_capacity[new_route[i] - 1];
            if (current_load > param.courier_capacity[courier_index - 1]) {
                return std::numeric_limits<double>::max();
            }
            current_time = std::max(
                current_time + param.location_distance_matrix[current_location][param.delivery_pickup_location[new_route[i] - 1]],
                param.delivery_release_time[new_route[i] - 1]
            );
            current_location = param.delivery_pickup_location[new_route[i] - 1];
        } else if (new_route[i] < 0) {
            current_load -= param.delivery_capacity[-new_route[i] - 1];
            current_time += param.location_distance_matrix[current_location][param.delivery_dropoff_location[-new_route[i] - 1]];
            if(current_time > sol.max_delivery_delivery_time){
                courier_attributed_delivery_time = std::numeric_limits<double>::max();
            }else{
                courier_attributed_delivery_time += current_time;
            }
            current_location = param.delivery_dropoff_location[-new_route[i] - 1];
        } else {
            break;
        }
    }
    return courier_attributed_delivery_time;
}

void apply_rerouting(const VRPPDParameters& param, VRPPDSolution& sol, int courier_index, const std::vector<int>& new_route) {
    sol.routing_plan[courier_index - 1] = new_route;
    double old_courier_attributed_delivery_time = sol.courier_attributed_delivery_time[courier_index - 1];
    sol.courier_attributed_delivery_time[courier_index - 1] = 0;
    double current_time = 0;
    sol.courier_current_load[courier_index - 1] = 0;
    int current_location = param.courier_starting_location[courier_index - 1];

    //std::cout << "Courier " << courier_index << " with associated delivery time " << sol.courier_attributed_delivery_time[courier_index - 1] << " rerouted" << std::endl;
    for (int i = 0; i < 2 * sol.delivery_count_assigned_to_courier[courier_index - 1]; ++i) {
        if (new_route[i] > 0) {
            sol.courier_current_load[courier_index - 1] += param.delivery_capacity[new_route[i] - 1];
            current_time = std::max(
                current_time + param.location_distance_matrix[current_location][param.delivery_pickup_location[new_route[i] - 1]],
                param.delivery_release_time[new_route[i] - 1]
            );
            current_location = param.delivery_pickup_location[new_route[i] - 1];
            //std::cout << "Delivery " << new_route[i] << " picked up at " << current_location << " at time " << current_time << std::endl;
        } else if (new_route[i] < 0) {
            sol.courier_current_load[courier_index - 1] -= param.delivery_capacity[-new_route[i] - 1];
            sol.delivery_delivery_time[-new_route[i] - 1] = current_time + param.location_distance_matrix[current_location][param.delivery_dropoff_location[-new_route[i] - 1]];
            //std::cout << "delivery time " << sol.delivery_delivery_time[-new_route[i] - 1] << " = " << current_time << " + " << param.location_distance_matrix[current_location][param.delivery_dropoff_location[-new_route[i] - 1]] << std::endl;
            current_time = sol.delivery_delivery_time[-new_route[i] - 1];
            current_location = param.delivery_dropoff_location[-new_route[i] - 1];
            sol.courier_attributed_delivery_time[courier_index - 1] += current_time;
            //std::cout << "Delivery " << -new_route[i] << " dropped off at " << current_location << " at time " << current_time << std::endl;
        }
    }
    //std::cout << "New associated delivery time of courier: " << sol.courier_attributed_delivery_time[courier_index - 1] << std::endl;
    
    sol.total_delivery_time = sol.total_delivery_time - old_courier_attributed_delivery_time + sol.courier_attributed_delivery_time[courier_index - 1];

}

void stack_courier_deliveries(const VRPPDParameters& param, VRPPDSolution& sol, int courier_index) {

    if (sol.delivery_count_assigned_to_courier[courier_index - 1] <= 1) {
        return;
    }

    std::vector<int> best_route = sol.routing_plan[courier_index - 1];
    double best_route_total_delivery_time = sol.courier_attributed_delivery_time[courier_index - 1];
    std::vector<int> incumbent_route(2 * sol.max_num_of_deliveries_assignable_to_courier, 0);
    std::vector<int> deliveries_in_route;
    for (int i = 0; i < 2 * sol.delivery_count_assigned_to_courier[courier_index - 1]; ++i) {
        if (sol.routing_plan[courier_index - 1][i] == 0) {
            throw std::runtime_error("Route is not properly formatted");
            break;
        } else if (sol.routing_plan[courier_index - 1][i] > 0) {
            deliveries_in_route.push_back(sol.routing_plan[courier_index - 1][i]);
        }
    }
    if (sol.delivery_count_assigned_to_courier[courier_index - 1] < 5) {
        std::vector<int> permutation = deliveries_in_route;
        do {
            for (const auto& combination : catalan_combinations(sol.delivery_count_assigned_to_courier[courier_index - 1])) {
                for (size_t i = 0; i < combination.size(); ++i) {
                    if (combination[i] > 0) {
                        incumbent_route[i] = permutation[combination[i] - 1];
                    } else {
                        incumbent_route[i] = -permutation[-combination[i] - 1];
                    }
                }
                double incumbent_route_total_delivery_time = delivery_time_of_rerouting(param, sol, courier_index, incumbent_route);
                if (incumbent_route_total_delivery_time <= best_route_total_delivery_time) {
                    best_route = incumbent_route;
                    best_route_total_delivery_time = incumbent_route_total_delivery_time;
                }
            }
        } while (std::next_permutation(permutation.begin(), permutation.end()));
    }else{
        throw std::runtime_error("Delivery count assigned to courier is too high for rerouting");
    }

    // Update the solution
    apply_rerouting(param, sol, courier_index, best_route);
}


void stack_all_courier_deliveries(const VRPPDParameters& param, VRPPDSolution& sol) {
    std::vector<int> former_routing_plan, new_routing_plan;

    for (int courier_index = 1; courier_index <= param.courier_count; ++courier_index) {
        stack_courier_deliveries(param, sol, courier_index);
    }
}

#endif