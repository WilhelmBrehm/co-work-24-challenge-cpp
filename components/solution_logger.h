#ifndef SOLUTION_LOGGER_CPP
#define SOLUTION_LOGGER_CPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


class SolutionLogger {
private:
    std::vector<double> entries;
    std::string log_file_path;

public:
    SolutionLogger(const std::string& file) : log_file_path(file) {}

    void add_entry(double entry) {
        entries.push_back(entry);
        if (entries.size() == entries.capacity()) {
            entries.reserve(entries.capacity() * 2);
        }
    }

    void write() {
        std::ofstream output_file(log_file_path);
        if (output_file.is_open()) {
            for (const auto& entry : entries) {
                output_file << entry << std::endl;
            }
            output_file.close();
        } else {
            std::cerr << "Failed to open log file." << std::endl;
        }
    }
};

#endif // SOLUTION_LOGGER_CPP