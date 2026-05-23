#ifndef AIZO_PROJECT2_BELLMAN_FORD_H
#define AIZO_PROJECT2_BELLMAN_FORD_H

#include <iostream>
#include <limits>

#include "graph/AdjacencyListGraph.h"
#include "graph/IncidenceMatrixGraph.h"
#include "graph/Edge.h"

class BellmanFord {
public:
    static void run(
    const AdjacencyListGraph& graph,
    const int start,
    const int end,
    const bool shouldPrintResult = true
    ) {
        const int vertexCount = graph.getVertexCount();

        if (!areVerticesValid(vertexCount, start, end)) {
            return;
        }

        int edgeCount = 0;
        Edge* edges = buildEdgesFromAdjacencyList(graph, edgeCount);

        runBellmanFord(vertexCount, edges, edgeCount, start, end, shouldPrintResult);

        delete[] edges;
    }

    static void run(
    const IncidenceMatrixGraph& graph,
    const int start,
    const int end,
    const bool shouldPrintResult = true
)    {
        const int vertexCount = graph.getVertexCount();

        if (!areVerticesValid(vertexCount, start, end)) {
            return;
        }

        int edgeCount = 0;
        Edge* edges = buildEdgesFromIncidenceMatrix(graph, edgeCount);

        runBellmanFord(vertexCount, edges, edgeCount, start, end, shouldPrintResult);

        delete[] edges;
    }

private:
    static bool areVerticesValid(const int vertexCount, const int start, const int end) {
        if (start < 0 || start >= vertexCount) {
            std::cout << "Error: invalid start vertex.\n";
            return false;
        }

        if (end < 0 || end >= vertexCount) {
            std::cout << "Error: invalid end vertex.\n";
            return false;
        }

        return true;
    }

    static Edge* buildEdgesFromAdjacencyList(
        const AdjacencyListGraph& graph,
        int& outputEdgeCount
    ) {
        const int vertexCount = graph.getVertexCount();

        int maxEdges = 0;

        for (int from = 0; from < vertexCount; ++from) {
            AdjNode* current = graph.getNeighbors(from);

            while (current != nullptr) {
                ++maxEdges;
                current = current->next;
            }
        }

        Edge* edges = new Edge[maxEdges];
        outputEdgeCount = 0;

        for (int from = 0; from < vertexCount; ++from) {
            AdjNode* current = graph.getNeighbors(from);

            while (current != nullptr) {
                edges[outputEdgeCount].from = from;
                edges[outputEdgeCount].to = current->to;
                edges[outputEdgeCount].weight = current->weight;

                ++outputEdgeCount;
                current = current->next;
            }
        }

        return edges;
    }

    static Edge* buildEdgesFromIncidenceMatrix(
        const IncidenceMatrixGraph& graph,
        int& outputEdgeCount
    ) {
        const int vertexCount = graph.getVertexCount();
        const int matrixEdgeCount = graph.getEdgeCount();

        Edge* edges = new Edge[matrixEdgeCount];
        outputEdgeCount = 0;

        for (int edge = 0; edge < matrixEdgeCount; ++edge) {
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
                edges[outputEdgeCount].from = from;
                edges[outputEdgeCount].to = to;
                edges[outputEdgeCount].weight = graph.getWeight(edge);
                ++outputEdgeCount;
            }
        }

        return edges;
    }

    static void runBellmanFord(
        const int vertexCount,
        const Edge* edges,
        const int edgeCount,
        const int start,
        const int end,
        const bool shouldPrintResult
    ) {
        int* distance = new int[vertexCount];
        int* previous = new int[vertexCount];

        initializeArrays(vertexCount, distance, previous);

        distance[start] = 0;

        for (int i = 0; i < vertexCount - 1; ++i) {
            bool changed = false;

            for (int edge = 0; edge < edgeCount; ++edge) {
                const int from = edges[edge].from;
                const int to = edges[edge].to;
                const int weight = edges[edge].weight;

                if (distance[from] == std::numeric_limits<int>::max()) {
                    continue;
                }

                if (distance[from] + weight < distance[to]) {
                    distance[to] = distance[from] + weight;
                    previous[to] = from;
                    changed = true;
                }
            }

            if (!changed) {
                break;
            }
        }

        if (hasNegativeCycle(edges, edgeCount, distance)) {
            if (shouldPrintResult) {
                std::cout << "Bellman-Ford result:\n";
                std::cout << "Negative cycle detected. Shortest path is not defined.\n";
            }

            delete[] distance;
            delete[] previous;
            return;
        }

        if (shouldPrintResult) {
            printResult(start, end, distance, previous);
        }

        delete[] distance;
        delete[] previous;
    }

    static void initializeArrays(
        const int vertexCount,
        int* distance,
        int* previous
    ) {
        for (int i = 0; i < vertexCount; ++i) {
            distance[i] = std::numeric_limits<int>::max();
            previous[i] = -1;
        }
    }

    static bool hasNegativeCycle(
        const Edge* edges,
        const int edgeCount,
        const int* distance
    ) {
        for (int edge = 0; edge < edgeCount; ++edge) {
            const int from = edges[edge].from;
            const int to = edges[edge].to;
            const int weight = edges[edge].weight;

            if (distance[from] == std::numeric_limits<int>::max()) {
                continue;
            }

            if (distance[from] + weight < distance[to]) {
                return true;
            }
        }

        return false;
    }

    static void printResult(
        const int start,
        const int end,
        const int* distance,
        const int* previous
    ) {
        std::cout << "Bellman-Ford result:\n";

        if (distance[end] == std::numeric_limits<int>::max()) {
            std::cout << "No path from " << start << " to " << end << ".\n";
            return;
        }

        std::cout << "Shortest path from " << start << " to " << end << ":\n";
        std::cout << "Cost: " << distance[end] << "\n";
        std::cout << "Path: ";

        printPath(previous, end);

        std::cout << "\n";
    }

    static void printPath(const int* previous, const int vertex) {
        if (previous[vertex] != -1) {
            printPath(previous, previous[vertex]);
            std::cout << " -> ";
        }

        std::cout << vertex;
    }
};

#endif //AIZO_PROJECT2_BELLMAN_FORD_H
