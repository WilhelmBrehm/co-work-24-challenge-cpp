#ifndef READ_DATA_H
#define READ_DATA_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

class Courier {
public:
    int courier_id;
    int location;
    int capacity;

    Courier(int id, int loc, int cap) : courier_id(id), location(loc), capacity(cap) {}
};

class Delivery {
public:
    int delivery_id;
    int capacity;
    int pickup_loc;
    int time_window_start;
    int pickup_stacking_id;
    int dropoff_loc;

    Delivery(int id, int cap, int pickup, int time_start, int stacking_id, int dropoff)
        : delivery_id(id), capacity(cap), pickup_loc(pickup), time_window_start(time_start),
          pickup_stacking_id(stacking_id), dropoff_loc(dropoff) {}
};

std::vector<Courier> load_couriers_from_csv(const std::string& filepath) {
    std::vector<Courier> couriers;
    std::ifstream file(filepath);
    std::string line;
    
    // Skip header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        std::vector<int> values;

        while (std::getline(iss, token, ',')) {
            values.push_back(std::stoi(token));
        }

        couriers.emplace_back(values[0], values[1], values[2]);
    }

    return couriers;
}

std::vector<Delivery> load_deliveries_from_csv(const std::string& filepath) {
    std::vector<Delivery> deliveries;
    std::ifstream file(filepath);
    std::string line;
    
    // Skip header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        std::vector<int> values;

        while (std::getline(iss, token, ',')) {
            values.push_back(std::stoi(token));
        }

        deliveries.emplace_back(values[0], values[1], values[2], values[3], values[4], values[5]);
    }

    return deliveries;
}

std::vector<std::vector<double> > load_travel_time_from_csv(const std::string& filepath) {
    std::vector<std::vector<double> > travel_time;
    std::ifstream file(filepath);
    std::string line;
    
    // Skip header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        std::vector<double> row;

        // Skip the first column (location index)
        std::getline(iss, token, ',');

        while (std::getline(iss, token, ',')) {
            row.push_back(std::stoi(token));
        }

        travel_time.push_back(row);
    }

    return travel_time;
}

struct VRPPDInstanceData {
    std::string instance_name;
    std::vector<Courier> couriers;
    std::vector<Delivery> deliveries;
    std::vector<std::vector<double> > travel_time;
};

VRPPDInstanceData process_instance_folder(const std::string& instance_folder_path) {
    std::string couriers_file, deliveries_file, travel_time_file;

    for (const auto& entry : std::filesystem::directory_iterator(instance_folder_path)) {
        std::string filename = entry.path().filename().string();
        if (filename.find("couriers.csv") != std::string::npos) {
            couriers_file = entry.path().string();
        } else if (filename.find("deliveries.csv") != std::string::npos) {
            deliveries_file = entry.path().string();
        } else if (filename.find("traveltimes.csv") != std::string::npos) {
            travel_time_file = entry.path().string();
        }
    }

    if (couriers_file.empty() || deliveries_file.empty() || travel_time_file.empty()) {
        throw std::runtime_error("Missing required CSV files in folder: " + instance_folder_path);
    }

    VRPPDInstanceData instance;
    instance.instance_name = std::filesystem::path(instance_folder_path).filename().string();
    instance.couriers = load_couriers_from_csv(couriers_file);
    instance.deliveries = load_deliveries_from_csv(deliveries_file);
    instance.travel_time = load_travel_time_from_csv(travel_time_file);

    return instance;
}

std::vector<VRPPDInstanceData> read_all_instances_from_folder(const std::string& parent_folder) {
    std::vector<VRPPDInstanceData> all_instances;

    for (const auto& entry : std::filesystem::directory_iterator(parent_folder)) {
        if (entry.is_directory()) {
            try {
                VRPPDInstanceData instance = process_instance_folder(entry.path().string());
                all_instances.push_back(instance);
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }
    }

    return all_instances;
}

#endif // READ_DATA_H