#ifndef IS_FEASIBLE_CPP
#define IS_FEASIBLE_CPP

#include "vrppd_solution.h"
#include "vrppd_parameters.h"
#include <iostream>

bool is_feasible(const VRPPDParameters& param, const VRPPDSolution& sol) {
    bool feasibility = sol.is_feasible_solution;

    if (!feasibility) {
        std::cout << "NO FEASIBLE SOLUTION FOUND" << std::endl;
        return feasibility;
    }

    // Check if every delivery is assigned to a courier
    for (int i = 0; i < param.delivery_count; ++i) {
        if (sol.delivery_assigned_courier[i] <= 0) {
            std::cout << "Delivery " << i+1 << " is not assigned to any courier" << std::endl;
            feasibility = false;
        }
    }
    //std::cout << "Delivery assignments checked" << std::endl;

    // Check if the assigned courier first picks up and then delivers the delivery
    for (int i = 1; i <= param.delivery_count; ++i) {
        int delivery_courier = sol.delivery_assigned_courier[i-1];

        bool is_picked_up = false;
        bool is_delivered = false;

        for (int j = 0; j < 2*sol.max_num_of_deliveries_assignable_to_courier; ++j) {
            if (sol.routing_plan[delivery_courier-1][j] == i) {
                is_picked_up = true;
            }
            if (sol.routing_plan[delivery_courier-1][j] == -i) {
                is_delivered = true;
            }
            if (is_delivered && !is_picked_up) {
                std::cout << "Delivery " << i << " is delivered before being picked up by courier " << delivery_courier << std::endl;
                feasibility = false;
            }
        }

        if (!is_picked_up) {
            std::cout << "Delivery " << i << " is not picked up by courier " << delivery_courier << std::endl;
            feasibility = false;
        }
        if (!is_delivered) {
            std::cout << "Delivery " << i << " is not delivered by courier " << delivery_courier << std::endl;
            feasibility = false;
        }
    }
    //std::cout << "Delivery order checked" << std::endl;

    // Check if the time window of each delivery is not violated
    for (int delivery_index = 0; delivery_index < param.delivery_count; delivery_index++) {

        int courier_index = sol.delivery_assigned_courier[delivery_index];
        double current_time = 0;
        double current_location = param.courier_starting_location[courier_index - 1];

        for(int j = 0; j < 2*sol.delivery_count_assigned_to_courier[courier_index - 1]; j++) {
            if(sol.routing_plan[courier_index - 1][j] > 0) {
                current_time += param.location_distance_matrix[current_location][param.delivery_pickup_location[sol.routing_plan[courier_index - 1][j] - 1]];
                if(current_time < param.delivery_release_time[sol.routing_plan[courier_index - 1][j] - 1]) {
                    current_time = param.delivery_release_time[sol.routing_plan[courier_index - 1][j] - 1];
                }
                current_location = param.delivery_pickup_location[sol.routing_plan[courier_index - 1][j] - 1];
            }else if(sol.routing_plan[courier_index - 1][j] < 0) {
                current_time += param.location_distance_matrix[current_location][param.delivery_dropoff_location[-sol.routing_plan[courier_index - 1][j] - 1]];
                if(sol.delivery_delivery_time[-sol.routing_plan[courier_index - 1][j] - 1] > current_time) {
                    std::cout << "Delivery " << -sol.routing_plan[courier_index - 1][j] << " is delivered before the courier can reach it" << std::endl;
                    feasibility = false;
                }
                current_location = param.delivery_dropoff_location[-sol.routing_plan[courier_index - 1][j] - 1];
            }

            
            if(sol.routing_plan[courier_index - 1][j] == -delivery_index) {
                if(current_time != sol.delivery_delivery_time[delivery_index - 1]) {
                    std::cout << "Time window of delivery " << delivery_index << " is violated" << std::endl;
                    std::cout << "Data delivery time: " << sol.delivery_delivery_time[delivery_index - 1] << ", computed delivery time: " << current_time <<  std::endl;
                    feasibility = false;
                }
                
            }
        }
    }
    //std::cout << "Time windows checked" << std::endl;

    // Check if the capacity of each courier is not exceeded
    for (int i = 0; i < param.courier_count; ++i) {
        int courier_capacity = 0;
        for (int j = 0; j < 2*sol.max_num_of_deliveries_assignable_to_courier; ++j) {
            if (sol.routing_plan[i][j] > 0) {
                courier_capacity += param.delivery_capacity[sol.routing_plan[i][j] - 1];
            }
            if (sol.routing_plan[i][j] < 0) {
                courier_capacity -= param.delivery_capacity[-sol.routing_plan[i][j] - 1];
            }
            if (courier_capacity > param.courier_capacity[i]) {
                std::cout << "Capacity of courier " << i+1 << " is exceeded" << std::endl;
                feasibility = false;
            }
        }
    }
    //std::cout << "Capacities checked" << std::endl;

    // Check if the travel time is feasible
    for (int i = 0; i < param.courier_count; ++i) {
        int current_location = param.courier_starting_location[i];
        double current_time = 0;
        for (int j = 0; j < 2*sol.max_num_of_deliveries_assignable_to_courier; ++j) {
            if (sol.routing_plan[i][j] > 0) {
                int delivery_pickup_location = param.delivery_pickup_location[sol.routing_plan[i][j] - 1];
                current_time += param.location_distance_matrix[current_location][delivery_pickup_location];
                if (current_time < param.delivery_release_time[sol.routing_plan[i][j] - 1]) {
                    current_time = param.delivery_release_time[sol.routing_plan[i][j] - 1];
                }
                current_location = delivery_pickup_location;
            }
            if (sol.routing_plan[i][j] < 0) {
                int delivery_dropoff_location = param.delivery_dropoff_location[-sol.routing_plan[i][j] - 1];
                current_time += param.location_distance_matrix[current_location][delivery_dropoff_location];
                if (sol.delivery_delivery_time[-sol.routing_plan[i][j] - 1] > current_time) {
                    std::cout << "Delivery " << -sol.routing_plan[i][j] << " is delivered before the courier can reach it" << std::endl;
                    feasibility = false;
                }
                current_location = delivery_dropoff_location;
            }
        }
    }
    //std::cout << "Travel times checked" << std::endl;

    // Check if the total delivery time equals the sum of delivery times
    double total_delivery_time = 0;
    for (int i = 0; i < param.delivery_count; ++i) {
        total_delivery_time += sol.delivery_delivery_time[i];
    }
        
    if (std::abs(total_delivery_time - sol.total_delivery_time) > 1e-6) {
        std::cout << "Total delivery time is not equal to sum of delivery times" << std::endl;
        std::cout << "Delivery times: ";
        for (double time : sol.delivery_delivery_time) {
            std::cout << time << " ";
        }
        std::cout << std::endl;
        std::cout << "Total delivery time: " << sol.total_delivery_time << std::endl;

        feasibility = false;
    }
    //std::cout << "Total delivery time checked" << std::endl;
    if(!feasibility) {
        std::cout << "SOLUTION INFEASIBLE" << std::endl;
    } 

    return feasibility;
}

#endif