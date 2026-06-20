#ifndef AIZO_PROJECT2_CSV_LOGGER_H
#define AIZO_PROJECT2_CSV_LOGGER_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>

struct BenchmarkResult {
    std::string date;
    std::string problem;
    std::string algorithm;
    std::string structure;

    int vertexCount;
    int edgeCount;
    double density;
    int iterations;

    long long minTimeUs;
    long long maxTimeUs;
    double avgTimeUs;
};

class CsvLogger {
    // Tworzy katalog nadrzedny pliku wynikowego
    static bool createParentDirectory(const std::string& filePath) {
        const std::filesystem::path path(filePath);
        const std::filesystem::path parentPath = path.parent_path();

        if (parentPath.empty()) {
            return true;
        }

        std::error_code error;

        std::filesystem::create_directories(parentPath, error);

        if (error) {
            std::cout
                << "Error: cannot create directory: "
                << parentPath.string()
                << ". Reason: "
                << error.message()
                << "\n";

            return false;
        }

        return true;
    }

public:

    // Zapisuje naglowek CSV
    static bool writeHeaderIfNeeded(const std::string& filePath) {
        if (!createParentDirectory(filePath)) {
            return false;
        }

        std::ifstream input(filePath);

        if (input.is_open()) {
            const bool fileIsEmpty =
                input.peek() == std::ifstream::traits_type::eof();

            input.close();

            if (!fileIsEmpty) {
                return true;
            }
        }

        std::ofstream output(
            filePath,
            std::ios::out | std::ios::trunc
        );

        if (!output.is_open()) {
            std::cout
                << "Error: cannot open CSV file for writing: "
                << filePath
                << "\n";

            return false;
        }

        output << "date;"
               << "problem;"
               << "algorithm;"
               << "structure;"
               << "vertexCount;"
               << "edgeCount;"
               << "density;"
               << "iterations;"
               << "minTimeUs;"
               << "maxTimeUs;"
               << "avgTimeUs\n";

        output.flush();

        if (!output.good()) {
            std::cout
                << "Error: cannot write CSV header to file: "
                << filePath
                << "\n";

            output.close();
            return false;
        }

        output.close();

        if (output.fail()) {
            std::cout
                << "Error: cannot close CSV file correctly: "
                << filePath
                << "\n";

            return false;
        }

        return true;
    }

    // Dopisuje jeden wynik benchmarku do pliku CSV
    static bool appendResult(
        const std::string& filePath,
        const BenchmarkResult& result
    ) {
        if (!writeHeaderIfNeeded(filePath)) {
            return false;
        }

        std::ofstream output(filePath, std::ios::app);

        if (!output.is_open()) {
            std::cout
                << "Error: cannot open CSV file for appending: "
                << filePath
                << "\n";

            return false;
        }

        output << result.date << ";"
               << result.problem << ";"
               << result.algorithm << ";"
               << result.structure << ";"
               << result.vertexCount << ";"
               << result.edgeCount << ";"
               << result.density << ";"
               << result.iterations << ";"
               << result.minTimeUs << ";"
               << result.maxTimeUs << ";"
               << result.avgTimeUs << "\n";

        output.flush();

        if (!output.good()) {
            std::cout
                << "Error: cannot write benchmark result to CSV file: "
                << filePath
                << "\n";

            output.close();
            return false;
        }

        output.close();

        if (output.fail()) {
            std::cout
                << "Error: cannot close CSV file correctly: "
                << filePath
                << "\n";

            return false;
        }

        return true;
    }
};

#endif // AIZO_PROJECT2_CSV_LOGGER_H