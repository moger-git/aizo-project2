#ifndef AIZO_PROJECT2_CSV_LOGGER_H
#define AIZO_PROJECT2_CSV_LOGGER_H

#include <fstream>
#include <iostream>
#include <string>

struct BenchmarkResult {
    std::string date;
    std::string problem;
    std::string algorithm;
    std::string structure;

    int vertexCount;
    int edgeCount;
    int density;
    int iterations;

    long long minTimeUs;
    long long maxTimeUs;
    double avgTimeUs;
};

class CsvLogger {
public:
    static bool writeHeaderIfNeeded(const std::string& filePath) {
        std::ifstream input(filePath);

        const bool fileExists = input.good();
        const bool fileIsEmpty = input.peek() == std::ifstream::traits_type::eof();

        input.close();

        if (fileExists && !fileIsEmpty) {
            return true;
        }

        std::ofstream output(filePath);

        if (!output.is_open()) {
            std::cout << "Error: cannot open CSV file: " << filePath << "\n";
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

        output.close();
        return true;
    }

    static bool appendResult(
        const std::string& filePath,
        const BenchmarkResult& result
    ) {
        if (!writeHeaderIfNeeded(filePath)) {
            return false;
        }

        std::ofstream output(filePath, std::ios::app);

        if (!output.is_open()) {
            std::cout << "Error: cannot open CSV file: " << filePath << "\n";
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

        output.close();
        return true;
    }
};

#endif //AIZO_PROJECT2_CSV_LOGGER_H
