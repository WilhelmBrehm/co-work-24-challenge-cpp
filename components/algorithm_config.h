#ifndef ALGORITHM_CONFIG_CPP
#define ALGORITHM_CONFIG_CPP

#include <iostream>
#include <fstream>
#include <sstream> 


struct AlgorithmConfig {
    int time_limit = 60;
    bool log_output = false;

    AlgorithmConfig(const std::string& file_path) {
        std::string line, key, value;
        std::ifstream ifs(file_path);
        if (ifs.is_open()) {
            while (ifs >> line) {
                replace(line.begin(), line.end(), '=', ' ');
                std::istringstream line_stream(line);
                line_stream >> key >> value;
                // std::cout << key << " " << value << std::endl;
                if (key == "time_limit") {
                    time_limit = std::stoi(value);
                } else if (key == "log_output") {
                    log_output = value == "true";
                }
            }
            ifs.close();
        } else {
            std::cout << "Failed to open file: " << file_path << std::endl;
        }
    }

};




#endif // ALGORITHM_CONFIG_CPP