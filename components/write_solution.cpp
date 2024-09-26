#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include "vrppd_solution.h"
#include "vrppd_parameters.h"

void write_solution_to_csv(const VRPPDParameters& param, const VRPPDSolution& solution, const std::string& output_file_path) {
    //std::cout << "Saving routing plan" << std::endl;
    std::ofstream output_file(output_file_path);
    output_file << "ID";

    if (!output_file.is_open()) {
        std::cerr << "Failed to open file: " << output_file_path << std::endl;
        return;
    }
    int courier_id = 1;
    // Process and write each row of the routing plan
    for (auto row : solution.routing_plan) {
        output_file << "\n";
        for(int i = 0; i < row.size(); i++){
            if(row[i] != 0) row[i] = std::abs(row[i]) + param.courier_count;
        }
        output_file << courier_id ;
        courier_id++;
        //if(!cleaned_row.empty()) {output_file << ",";}
        // Write the cleaned row to the CSV file
        for (int i = 0; i < row.size(); ++i) {
            if (row[i] != 0) {
                output_file << "," << row[i];
            }
        }
    }

    output_file.close();

    std::cout << "Routing successfully plan saved to " << output_file_path << std::endl;
}