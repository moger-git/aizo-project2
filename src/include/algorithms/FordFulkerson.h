#ifndef AIZO_PROJECT2_FORD_FULKERSON_H
#define AIZO_PROJECT2_FORD_FULKERSON_H

#include <iostream>
#include <limits>

#include "graph/AdjacencyListGraph.h"
#include "graph/IncidenceMatrixGraph.h"

class FordFulkerson {
public:
    static void run(
    const AdjacencyListGraph& graph,
    const int source,
    const int sink,
    const bool shouldPrintResult = true
    ) {
        const int vertexCount = graph.getVertexCount();

        if (!areVerticesValid(vertexCount, source, sink, shouldPrintResult)) {
            return;
        }

        int** capacity = createMatrix(vertexCount);
        fillCapacityFromAdjacencyList(graph, capacity);

        runFordFulkerson(capacity, vertexCount, source, sink, shouldPrintResult);

        deleteMatrix(capacity, vertexCount);
    }

    static void run(
    const IncidenceMatrixGraph& graph,
    const int source,
    const int sink,
    const bool shouldPrintResult = true
    ) {
        const int vertexCount = graph.getVertexCount();

        if (!areVerticesValid(vertexCount, source, sink, shouldPrintResult)) {
            return;
        }

        int** capacity = createMatrix(vertexCount);
        fillCapacityFromIncidenceMatrix(graph, capacity);

        runFordFulkerson(capacity, vertexCount, source, sink, shouldPrintResult);

        deleteMatrix(capacity, vertexCount);
    }

private:
    static bool areVerticesValid(
    const int vertexCount,
    const int source,
    const int sink,
    const bool shouldPrintResult
    ) {
        if (source < 0 || source >= vertexCount) {
            if (shouldPrintResult) {
                std::cout << "Error: invalid source vertex.\n";
            }

            return false;
        }

        if (sink < 0 || sink >= vertexCount) {
            std::cout << "Error: invalid sink vertex.\n";
            return false;
        }

        if (source == sink) {
            std::cout << "Error: source and sink cannot be the same.\n";
            return false;
        }

        return true;
    }

    static int** createMatrix(const int size) {
        int** matrix = new int*[size];

        for (int i = 0; i < size; ++i) {
            matrix[i] = new int[size];

            for (int j = 0; j < size; ++j) {
                matrix[i][j] = 0;
            }
        }

        return matrix;
    }

    static void deleteMatrix(int** matrix, const int size) {
        for (int i = 0; i < size; ++i) {
            delete[] matrix[i];
        }

        delete[] matrix;
    }

    static void fillCapacityFromAdjacencyList(
        const AdjacencyListGraph& graph,
        int** capacity
    ) {
        const int vertexCount = graph.getVertexCount();

        for (int from = 0; from < vertexCount; ++from) {
            AdjNode* current = graph.getNeighbors(from);

            while (current != nullptr) {
                const int to = current->to;
                const int flowCapacity = current->weight;

                capacity[from][to] += flowCapacity;

                current = current->next;
            }
        }
    }

    static void fillCapacityFromIncidenceMatrix(
        const IncidenceMatrixGraph& graph,
        int** capacity
    ) {
        const int vertexCount = graph.getVertexCount();
        const int edgeCount = graph.getEdgeCount();

        for (int edge = 0; edge < edgeCount; ++edge) {
            int from = -1;
            int to = -1;

            for (int vertex = 0; vertex < vertexCount; ++vertex) {
                if (graph.getMatrixValue(vertex, edge) == -1) {
                    from = vertex;
                } else if (graph.getMatrixValue(vertex, edge) == 1) {
                    to = vertex;
                }
            }

            if (from != -1 && to != -1) {
                capacity[from][to] += graph.getWeight(edge);
            }
        }
    }

    static void runFordFulkerson(
    int** capacity,
    const int vertexCount,
    const int source,
    const int sink,
    const bool shouldPrintResult
    ) {
        int** residual = createMatrix(vertexCount);

        for (int i = 0; i < vertexCount; ++i) {
            for (int j = 0; j < vertexCount; ++j) {
                residual[i][j] = capacity[i][j];
            }
        }

        int* parent = new int[vertexCount];
        int maxFlow = 0;

        while (bfs(residual, vertexCount, source, sink, parent)) {
            int pathFlow = std::numeric_limits<int>::max();

            int current = sink;

            while (current != source) {
                const int previous = parent[current];

                if (residual[previous][current] < pathFlow) {
                    pathFlow = residual[previous][current];
                }

                current = previous;
            }

            current = sink;

            while (current != source) {
                const int previous = parent[current];

                residual[previous][current] -= pathFlow;
                residual[current][previous] += pathFlow;

                current = previous;
            }

            maxFlow += pathFlow;

            if (shouldPrintResult) {
                std::cout << "Augmenting path flow: " << pathFlow << "\n";
            }
        }

        if (shouldPrintResult) {
            std::cout << "Ford-Fulkerson result:\n";
            std::cout << "Maximum flow from "
                      << source
                      << " to "
                      << sink
                      << ": "
                      << maxFlow
                      << "\n";
        }

        delete[] parent;
        deleteMatrix(residual, vertexCount);
    }

    static bool bfs(
        int** residual,
        const int vertexCount,
        const int source,
        const int sink,
        int* parent
    ) {
        bool* visited = new bool[vertexCount];

        for (int i = 0; i < vertexCount; ++i) {
            visited[i] = false;
            parent[i] = -1;
        }

        int* queue = new int[vertexCount];
        int queueStart = 0;
        int queueEnd = 0;

        queue[queueEnd] = source;
        ++queueEnd;

        visited[source] = true;

        while (queueStart < queueEnd) {
            const int current = queue[queueStart];
            ++queueStart;

            for (int next = 0; next < vertexCount; ++next) {
                if (!visited[next] && residual[current][next] > 0) {
                    parent[next] = current;
                    visited[next] = true;

                    if (next == sink) {
                        delete[] queue;
                        delete[] visited;
                        return true;
                    }

                    queue[queueEnd] = next;
                    ++queueEnd;
                }
            }
        }

        delete[] queue;
        delete[] visited;
        return false;
    }
};

#endif //AIZO_PROJECT2_FORD_FULKERSON_H
