#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <cmath>
#include "components/vrppd_parameters.h"
#include "components/vrppd_solution.h"
#include "components/stack_courier_deliveries.cpp"
#include "components/heuristic_generator.cpp"
#include "components/is_feasible.cpp"
#include "components/read_data.cpp"
#include "components/write_solution.cpp"
#include "components/algorithm_config.h"
#include "components/solution_logger.h"



int main(int argc, char *argv[]) {
    std::string path_to_problem_parameters   = argv[1],
                path_to_solver_parameters    = argv[2],
                path_for_solution_file       = argv[3];

    VRPPDInstanceData instance_data = process_instance_folder(path_to_problem_parameters);
    VRPPDParameters param(instance_data);

    AlgorithmConfig algorithm_config(path_to_solver_parameters);

    VRPPDSolution best_solution(param.courier_count, param.delivery_count),
                  incumbent_solution(param.courier_count, param.delivery_count);

    if (best_solution.max_num_of_deliveries_assignable_to_courier < param.delivery_count / param.courier_count) {
        throw std::runtime_error("The number of couriers per delivery is too low");
    }

    auto time_start = std::chrono::high_resolution_clock::now();
    int iteration = 0;
    // std::cout << "Starting iterations" << std::endl;
    while(true){
        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - time_start).count() >= algorithm_config.time_limit) break;
        //if(iteration >= 1000) break;
        //std::cout << "Iteration " << iteration << ": start ";
        iteration++;
        incumbent_solution = VRPPDSolution(param.courier_count, param.delivery_count);
        random_greedy_courier_heuristic(param, incumbent_solution);
        // if(incumbent_solution.is_feasible_solution) stack_all_courier_deliveries(param, incumbent_solution);
        //std::cout << " -> 3" << std::endl;
        if (incumbent_solution.total_delivery_time < best_solution.total_delivery_time) {
            best_solution = incumbent_solution;
        }
        //std::cout << "Iteration " << iteration << " done" << std::endl;
    }
    std::cout << "Iterations: " << iteration << " done" << std::endl;


    if (!is_feasible(param, best_solution) || !best_solution.is_feasible_solution) { 
        //std::cout << "No Solution for Instance " << path_to_problem_parameters << " found" << std::endl;
        throw std::runtime_error("Solution is not feasible");
    }else{
        //std::cout << "Best solution found: " << best_solution.total_delivery_time << " after " << iteration << " iterations" << std::endl;
    }

    write_solution_to_csv(param, best_solution, path_for_solution_file+".csv");
    //if(algorithm_config.log_output) solution_logger.write();

    return 0;
}