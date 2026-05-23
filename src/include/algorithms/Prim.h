#ifndef AIZO_PROJECT2_PRIM_H
#define AIZO_PROJECT2_PRIM_H

#include <iostream>
#include <limits>

#include "graph/AdjacencyListGraph.h"
#include "graph/IncidenceMatrixGraph.h"

class Prim {
public:
    static void run(
    const AdjacencyListGraph& graph,
    const bool shouldPrintResult = true
    ) {
        const int vertexCount = graph.getVertexCount();

        if (vertexCount <= 0) {
            std::cout << "Prim result:\n";
            std::cout << "Graph is empty.\n";
            return;
        }

        int* key = new int[vertexCount];
        int* parent = new int[vertexCount];
        bool* inMst = new bool[vertexCount];

        initializeArrays(vertexCount, key, parent, inMst);

        key[0] = 0;

        for (int i = 0; i < vertexCount; ++i) {
            const int current = findMinimumKeyVertex(vertexCount, key, inMst);

            if (current == -1) {
                break;
            }

            inMst[current] = true;

            AdjNode* neighbor = graph.getNeighbors(current);

            while (neighbor != nullptr) {
                const int to = neighbor->to;
                const int weight = neighbor->weight;

                if (!inMst[to] && weight < key[to]) {
                    key[to] = weight;
                    parent[to] = current;
                }

                neighbor = neighbor->next;
            }
        }

        if (shouldPrintResult) {
            printResult(vertexCount, key, parent);
        }

        delete[] key;
        delete[] parent;
        delete[] inMst;
    }

    static void run(
    const IncidenceMatrixGraph& graph,
    const bool shouldPrintResult = true
    ) {
        const int vertexCount = graph.getVertexCount();
        const int edgeCount = graph.getEdgeCount();

        if (vertexCount <= 0) {
            if (shouldPrintResult) {
                std::cout << "Prim result:\n";
                std::cout << "Graph is empty.\n";
            }

            return;
        }

        int* key = new int[vertexCount];
        int* parent = new int[vertexCount];
        bool* inMst = new bool[vertexCount];

        initializeArrays(vertexCount, key, parent, inMst);

        key[0] = 0;

        for (int i = 0; i < vertexCount; ++i) {
            const int current = findMinimumKeyVertex(vertexCount, key, inMst);

            if (current == -1) {
                break;
            }

            inMst[current] = true;

            for (int edge = 0; edge < edgeCount; ++edge) {
                if (graph.getMatrixValue(current, edge) == 1) {
                    const int to = findOtherVertex(graph, edge, current);

                    if (to != -1) {
                        const int weight = graph.getWeight(edge);

                        if (!inMst[to] && weight < key[to]) {
                            key[to] = weight;
                            parent[to] = current;
                        }
                    }
                }
            }
        }

        if (shouldPrintResult) {
            printResult(vertexCount, key, parent);
        }

        delete[] key;
        delete[] parent;
        delete[] inMst;
    }

private:
    static void initializeArrays(
        const int vertexCount,
        int* key,
        int* parent,
        bool* inMst
    ) {
        for (int i = 0; i < vertexCount; ++i) {
            key[i] = std::numeric_limits<int>::max();
            parent[i] = -1;
            inMst[i] = false;
        }
    }

    static int findMinimumKeyVertex(
        const int vertexCount,
        const int* key,
        const bool* inMst
    ) {
        int minVertex = -1;

        for (int i = 0; i < vertexCount; ++i) {
            if (!inMst[i] &&
                (minVertex == -1 || key[i] < key[minVertex])) {
                minVertex = i;
            }
        }

        return minVertex;
    }

    static int findOtherVertex(
        const IncidenceMatrixGraph& graph,
        const int edge,
        const int current
    ) {
        const int vertexCount = graph.getVertexCount();

        for (int vertex = 0; vertex < vertexCount; ++vertex) {
            if (vertex != current && graph.getMatrixValue(vertex, edge) == 1) {
                return vertex;
            }
        }

        return -1;
    }

    static void printResult(
        const int vertexCount,
        const int* key,
        const int* parent
    ) {
        int totalCost = 0;
        bool connected = true;

        std::cout << "Prim result:\n";
        std::cout << "Edges in MST:\n";

        for (int vertex = 1; vertex < vertexCount; ++vertex) {
            if (parent[vertex] == -1) {
                connected = false;
                continue;
            }

            std::cout << parent[vertex]
                      << " - "
                      << vertex
                      << " weight "
                      << key[vertex]
                      << "\n";

            totalCost += key[vertex];
        }

        if (!connected) {
            std::cout << "Warning: graph is not connected. MST does not include all vertices.\n";
        }

        std::cout << "MST cost: " << totalCost << "\n";
    }
};

#endif //AIZO_PROJECT2_PRIM_H
