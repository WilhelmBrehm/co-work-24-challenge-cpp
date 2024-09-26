#ifndef HEURISTIC_GENERATOR_CPP
#define HEURISTIC_GENERATOR_CPP

#include "vrppd_solution.h"
#include "vrppd_parameters.h"
#include <random>
#include <algorithm>
#include <cmath>

struct CourierState {
    int courier_index;
    int current_location;
    double current_time;

    CourierState(int index, int location, double time)
        : courier_index(index), current_location(location), current_time(time) {}
};

struct CourierMove {
    int courier_index;
    int delivery_index;
    double cost;
    bool move_appends;

    CourierMove(int c_index, int d_index, double c, bool appends = true)
        : courier_index(c_index), delivery_index(d_index), cost(c), move_appends(appends) {}
};

double append_delivery_delivery_time(const VRPPDParameters& param, const VRPPDSolution& sol, const CourierState& courier_state, const int delivery_index) {
    int delivery_pickup_location = param.delivery_pickup_location[delivery_index - 1];
    int delivery_dropoff_location = param.delivery_dropoff_location[delivery_index - 1];
    double delivery_time = std::max(
        courier_state.current_time + param.location_distance_matrix[courier_state.current_location][delivery_pickup_location],
        param.delivery_release_time[delivery_index - 1]
    ) + param.location_distance_matrix[delivery_pickup_location][delivery_dropoff_location];
    return delivery_time;
}

void apply_courier_move(const VRPPDParameters& param, VRPPDSolution& sol, CourierState& courier_state, const CourierMove& courier_move) {
    if (courier_move.move_appends) {
        int delivery_dropoff_location = param.delivery_dropoff_location[courier_move.delivery_index - 1];
        double delivery_delivery_time = append_delivery_delivery_time(param, sol, courier_state, courier_move.delivery_index);
        courier_state.current_location = delivery_dropoff_location;
        courier_state.current_time = delivery_delivery_time;
        sol.total_delivery_time += delivery_delivery_time;
        sol.routing_plan[courier_move.courier_index - 1][2*sol.delivery_count_assigned_to_courier[courier_move.courier_index - 1]] = courier_move.delivery_index;
        sol.routing_plan[courier_move.courier_index - 1][2*sol.delivery_count_assigned_to_courier[courier_move.courier_index - 1] + 1] = -courier_move.delivery_index;
        sol.delivery_assigned_courier[courier_move.delivery_index - 1] = courier_move.courier_index;
        sol.delivery_count_assigned_to_courier[courier_move.courier_index - 1]++;
        sol.delivery_delivery_time[courier_move.delivery_index - 1] = delivery_delivery_time;
        sol.courier_attributed_delivery_time[courier_move.courier_index - 1] += delivery_delivery_time;
    } else {
        throw std::runtime_error("Non-appending moves not implemented");
    }
}

class GreedyDeliveryFinder {
private:
    std::vector<int> count_of_already_considered_closest_deliveries_from_location;

public:
    GreedyDeliveryFinder(const VRPPDParameters& param)
        : count_of_already_considered_closest_deliveries_from_location(param.location_count, 0) {}

    CourierMove greedy_delivery_of_courier(const VRPPDParameters& param, const VRPPDSolution& sol, const CourierState& courier_state) {
        int current_location = courier_state.current_location;
        for (int i = count_of_already_considered_closest_deliveries_from_location[current_location]; i < param.delivery_count; ++i) {
            int closest_delivery = param.location_nearest_delivery_matrix[current_location][i];
            if(
                sol.delivery_count_assigned_to_courier[courier_state.courier_index - 1] >= sol.max_num_of_deliveries_assignable_to_courier
            ){
                count_of_already_considered_closest_deliveries_from_location[current_location] = param.delivery_count;
                break;
            } else if (sol.delivery_assigned_courier[closest_delivery - 1] > 0 || 
                param.delivery_capacity[closest_delivery - 1] > param.courier_capacity[courier_state.courier_index - 1]) {
                count_of_already_considered_closest_deliveries_from_location[current_location]++;
            }else {
                double delivery_time = append_delivery_delivery_time(param, sol, courier_state, closest_delivery);
                if(courier_state.current_time + delivery_time > sol.max_delivery_delivery_time) {
                    count_of_already_considered_closest_deliveries_from_location[current_location] = param.delivery_count;
                    break;
                }
                return CourierMove(courier_state.courier_index, closest_delivery, delivery_time, true);
            }
        }
        return CourierMove(courier_state.courier_index, 0, std::numeric_limits<double>::infinity(), false);
    }
};

void random_greedy_courier_heuristic(const VRPPDParameters& param, VRPPDSolution& sol) {
    sol.total_delivery_time = 0;
    
    GreedyDeliveryFinder greedy_delivery_finder_singleton(param);

    std::vector<CourierState> courier_states;
    for (int courier_index = 1; courier_index <= param.courier_count; ++courier_index) {
        courier_states.emplace_back(courier_index, param.courier_starting_location[courier_index - 1], 0);
    }

    int iteration = 0;
    int assigned_deliveries = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    std::vector<CourierMove> greedy_courier_moves;
    CourierMove greedy_courier_move(0, 0, 0, false);
    //std::cout << "Starting iterations 2" << std::endl;
    while (assigned_deliveries < param.delivery_count) {
        iteration++;
        
        greedy_courier_moves.clear();
        for (const auto& courier_state : courier_states) {
            greedy_courier_move = greedy_delivery_finder_singleton.greedy_delivery_of_courier(param, sol, courier_state);
            if (greedy_courier_move.delivery_index > 0) {greedy_courier_moves.push_back(greedy_courier_move);}
        }
        std::sort(greedy_courier_moves.begin(), greedy_courier_moves.end(),
                  [](const CourierMove& a, const CourierMove& b) { return a.cost < b.cost; });

        std::vector<double> random_probability_vector(param.courier_count);
        for (int i = 0; i < param.courier_count; ++i) {
            random_probability_vector[i] = dis(gen);
        }

        for (const auto& courier_move : greedy_courier_moves) {
            if (assigned_deliveries == param.delivery_count) break;
            double prob = random_probability_vector[courier_move.courier_index - 1];
            prob = prob * prob;
            if (prob <= static_cast<double>(assigned_deliveries + 1) / param.delivery_count) {
                if (sol.delivery_assigned_courier[courier_move.delivery_index - 1] == 0) {
                    sol.delivery_assigned_courier[courier_move.delivery_index - 1] = courier_move.courier_index;
                    assigned_deliveries++;
                    apply_courier_move(param, sol, courier_states[courier_move.courier_index - 1], courier_move);
                }
            }
        }
        if(iteration > 10000) {
            sol.total_delivery_time = std::numeric_limits<double>::max();
            sol.is_feasible_solution = false;
            return;
        }
    }
    sol.is_feasible_solution = true;
}

#endif // HEURISTIC_GENERATOR_CPP