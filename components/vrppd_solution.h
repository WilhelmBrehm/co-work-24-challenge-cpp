#ifndef VRPPD_SOLUTION_H
#define VRPPD_SOLUTION_H

#include <vector>


struct VRPPDSolution {
    bool is_feasible_solution = false;
    int max_num_of_deliveries_assignable_to_courier = 4;
    int max_delivery_delivery_time = 180;
    double total_delivery_time;
    std::vector<std::vector<int> > routing_plan;
    std::vector<int> delivery_count_assigned_to_courier;
    std::vector<double> delivery_delivery_time;
    std::vector<int> delivery_assigned_courier;
    std::vector<double> courier_attributed_delivery_time;
    std::vector<double> courier_current_load;

    VRPPDSolution(int courier_count, int delivery_count)
        : total_delivery_time(std::numeric_limits<double>::max()),
          routing_plan(courier_count, std::vector<int>(2*max_num_of_deliveries_assignable_to_courier, 0)),
          delivery_count_assigned_to_courier(courier_count, 0),
          delivery_delivery_time(delivery_count, std::numeric_limits<double>::max()),
          delivery_assigned_courier(delivery_count, 0),
          courier_attributed_delivery_time(courier_count, 0),
          courier_current_load(courier_count, 0.0) {}
};

#endif // VRPPD_SOLUTION_H
