#ifndef AIZO_PROJECT2_INCIDENCEMATRIXGRAPH_H
#define AIZO_PROJECT2_INCIDENCEMATRIXGRAPH_H
#include <iostream>

class IncidenceMatrixGraph {
    int vertexCount;
    int edgeCapacity;
    int edgeCount;
    int** matrix;
    int* weights;

public:
    IncidenceMatrixGraph(int vertices, int edges)
        : vertexCount(vertices), edgeCapacity(edges), edgeCount(0) {
        matrix = new int*[vertexCount];

        for (int i = 0; i < vertexCount; ++i) {
            matrix[i] = new int[edgeCapacity];

            for (int j = 0; j < edgeCapacity; ++j) {
                matrix[i][j] = 0;
            }
        }

        weights = new int[edgeCapacity];

        for (int i = 0; i < edgeCapacity; ++i) {
            weights[i] = 0;
        }
    }

    ~IncidenceMatrixGraph() {
        for (int i = 0; i < vertexCount; ++i) {
            delete[] matrix[i];
        }

        delete[] matrix;
        delete[] weights;
    }

    void addDirectedEdge(int from, int to, int weight) {
        if (edgeCount >= edgeCapacity) {
            return;
        }

        matrix[from][edgeCount] = -1;
        matrix[to][edgeCount] = 1;
        weights[edgeCount] = weight;
        ++edgeCount;
    }

    void addUndirectedEdge(int from, int to, int weight) {
        if (edgeCount >= edgeCapacity) {
            return;
        }

        matrix[from][edgeCount] = 1;
        matrix[to][edgeCount] = 1;
        weights[edgeCount] = weight;
        ++edgeCount;
    }

    int getVertexCount() const {
        return vertexCount;
    }

    int getEdgeCount() const {
        return edgeCount;
    }

    int getMatrixValue(const int vertex, const int edge) const {
        return matrix[vertex][edge];
    }

    int getWeight(const int edge) const {
        return weights[edge];
    }

    void print() const {
        std::cout << "Incidence matrix:\n";

        for (int i = 0; i < vertexCount; ++i) {
            for (int j = 0; j < edgeCount; ++j) {
                std::cout << matrix[i][j] << " ";
            }

            std::cout << "\n";
        }
    }
};

#endif //AIZO_PROJECT2_INCIDENCEMATRIXGRAPH_H
