#ifndef AIZO_PROJECT2_GRAPH_LOADER_H
#define AIZO_PROJECT2_GRAPH_LOADER_H

#include <fstream>
#include <iostream>
#include <string>

#include "graph/Edge.h"
#include "graph/GraphData.h"


class GraphLoader {
public:
    static bool load(const std::string& path, GraphData& data) {
        std::ifstream file(path);

        if (!file.is_open()) {
            std::cout << "Error: cannot open graph file: " << path << "\n";
            return false;
        }

        file >> data.vertexCount >> data.edgeCount;

        if (!file || data.vertexCount <= 0 || data.edgeCount < 0) {
            std::cout << "Error: invalid first line in graph file.\n";
            return false;
        }

        data.edges = new Edge[data.edgeCount];

        for (int i = 0; i < data.edgeCount; ++i) {
            file >> data.edges[i].from >> data.edges[i].to >> data.edges[i].weight;

            if (!file) {
                std::cout << "Error: invalid edge line.\n";
                delete[] data.edges;
                data.edges = nullptr;
                return false;
            }

            if (data.edges[i].from < 0 || data.edges[i].from >= data.vertexCount ||
                data.edges[i].to < 0 || data.edges[i].to >= data.vertexCount ||
                data.edges[i].weight < 0) {
                std::cout << "Error: edge has invalid values.\n";
                delete[] data.edges;
                data.edges = nullptr;
                return false;
            }
        }

        return true;
    }

    static void free(GraphData& data) {
        delete[] data.edges;
        data.edges = nullptr;
        data.edgeCount = 0;
        data.vertexCount = 0;
    }
};

#endif //AIZO_PROJECT2_GRAPH_LOADER_H
