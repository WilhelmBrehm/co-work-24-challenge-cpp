#ifndef VRPPD_PARAMETERS_H
#define VRPPD_PARAMETERS_H

#include <vector>
#include <string>
#include "read_data.cpp"

struct VRPPDParameters {
    int delivery_count;
    std::vector<int> delivery_capacity;
    std::vector<double> delivery_release_time;
    std::vector<int> delivery_pickup_location;
    std::vector<int> delivery_dropoff_location;

    int courier_count;
    std::vector<int> courier_capacity;
    std::vector<int> courier_starting_location;

    int location_count;
    std::vector<std::vector<double> > location_distance_matrix;
    std::vector<std::vector<int> > location_nearest_location_matrix;
    std::vector<std::vector<int> > location_nearest_delivery_matrix;

    VRPPDParameters(const VRPPDInstanceData& instance) {
        delivery_count = instance.deliveries.size();
        delivery_capacity.reserve(delivery_count);
        delivery_release_time.reserve(delivery_count);
        delivery_pickup_location.reserve(delivery_count);
        delivery_dropoff_location.reserve(delivery_count);

        for (const auto& delivery : instance.deliveries) {
            delivery_capacity.push_back(delivery.capacity);
            delivery_release_time.push_back(delivery.time_window_start);
            delivery_pickup_location.push_back(delivery.pickup_loc - 1);
            delivery_dropoff_location.push_back(delivery.dropoff_loc - 1);
        }

        courier_count = instance.couriers.size();
        courier_capacity.reserve(courier_count);
        courier_starting_location.reserve(courier_count);

        for (const auto& courier : instance.couriers) {
            courier_capacity.push_back(courier.capacity);
            courier_starting_location.push_back(courier.location - 1);
        }

        location_count = instance.travel_time.size();
        location_distance_matrix = instance.travel_time;

        // Initialize location_nearest_location_matrix
        location_nearest_location_matrix.resize(location_count);
        for (int i = 0; i < location_count; ++i) {
            std::vector<int> indices(location_count);
            for (int j = 0; j < location_count; ++j) indices[j] = j;
            std::sort(indices.begin(), indices.end(),
                [&](int a, int b) { return location_distance_matrix[i][a] < location_distance_matrix[i][b]; });
            indices.erase(std::remove(indices.begin(), indices.end(), i), indices.end());
            location_nearest_location_matrix[i] = indices;
        }

        // Initialize location_nearest_delivery_matrix
        location_nearest_delivery_matrix.resize(location_count);
        for (int i = 0; i < location_count; ++i) {
            std::vector<std::pair<int, double>> delivery_distances;
            for (int j = 0; j < delivery_count; ++j) {
                delivery_distances.emplace_back(j, location_distance_matrix[i][delivery_pickup_location[j]]);
            }
            std::sort(delivery_distances.begin(), delivery_distances.end(),
                [](const auto& a, const auto& b) { return a.second < b.second; });
            for (const auto& pair : delivery_distances) {
                location_nearest_delivery_matrix[i].push_back(pair.first + 1);
            }
        }
    }
};

#endif // VRPPD_PARAMETERS_H
