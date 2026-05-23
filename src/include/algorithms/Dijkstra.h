#ifndef AIZO_PROJECT2_DIJKSTRA_H
#define AIZO_PROJECT2_DIJKSTRA_H

#include <iostream>
#include <limits>

#include "graph/AdjacencyListGraph.h"
#include "graph/IncidenceMatrixGraph.h"

class Dijkstra {
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

        int* distance = new int[vertexCount];
        bool* visited = new bool[vertexCount];
        int* previous = new int[vertexCount];

        initializeArrays(vertexCount, distance, visited, previous);

        distance[start] = 0;

        for (int i = 0; i < vertexCount; ++i) {
            const int current = findNearestUnvisitedVertex(vertexCount, distance, visited);

            if (current == -1) {
                break;
            }

            if (distance[current] == std::numeric_limits<int>::max()) {
                break;
            }

            visited[current] = true;

            AdjNode* neighbor = graph.getNeighbors(current);

            while (neighbor != nullptr) {
                const int to = neighbor->to;
                const int weight = neighbor->weight;

                relaxEdge(current, to, weight, distance, previous);

                neighbor = neighbor->next;
            }
        }

        if (shouldPrintResult) {
            printResult(start, end, distance, previous);
        }

        delete[] distance;
        delete[] visited;
        delete[] previous;
    }

    static void run(
        const IncidenceMatrixGraph& graph,
        const int start,
        const int end,
        const bool shouldPrintResult = true
        ) {
        const int vertexCount = graph.getVertexCount();
        const int edgeCount = graph.getEdgeCount();

        if (!areVerticesValid(vertexCount, start, end)) {
            return;
        }

        int* distance = new int[vertexCount];
        bool* visited = new bool[vertexCount];
        int* previous = new int[vertexCount];

        initializeArrays(vertexCount, distance, visited, previous);

        distance[start] = 0;

        for (int i = 0; i < vertexCount; ++i) {
            const int current = findNearestUnvisitedVertex(vertexCount, distance, visited);

            if (current == -1) {
                break;
            }

            if (distance[current] == std::numeric_limits<int>::max()) {
                break;
            }

            visited[current] = true;

            for (int edge = 0; edge < edgeCount; ++edge) {
                if (graph.getMatrixValue(current, edge) == -1) {
                    const int to = findDirectedEdgeEnd(graph, edge, current);

                    if (to != -1) {
                        const int weight = graph.getWeight(edge);
                        relaxEdge(current, to, weight, distance, previous);
                    }
                }
            }
        }

        if (shouldPrintResult) {
            printResult(start, end, distance, previous);
        }

        delete[] distance;
        delete[] visited;
        delete[] previous;
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

    static void initializeArrays(
        const int vertexCount,
        int* distance,
        bool* visited,
        int* previous
    ) {
        for (int i = 0; i < vertexCount; ++i) {
            distance[i] = std::numeric_limits<int>::max();
            visited[i] = false;
            previous[i] = -1;
        }
    }

    static int findNearestUnvisitedVertex(
        const int vertexCount,
        const int* distance,
        const bool* visited
    ) {
        int nearestVertex = -1;

        for (int i = 0; i < vertexCount; ++i) {
            if (!visited[i] &&
                (nearestVertex == -1 || distance[i] < distance[nearestVertex])) {
                nearestVertex = i;
            }
        }

        return nearestVertex;
    }

    static void relaxEdge(
        const int from,
        const int to,
        const int weight,
        int* distance,
        int* previous
    ) {
        if (distance[from] == std::numeric_limits<int>::max()) {
            return;
        }

        if (distance[from] + weight < distance[to]) {
            distance[to] = distance[from] + weight;
            previous[to] = from;
        }
    }

    static int findDirectedEdgeEnd(
        const IncidenceMatrixGraph& graph,
        const int edge,
        const int from
    ) {
        const int vertexCount = graph.getVertexCount();

        for (int vertex = 0; vertex < vertexCount; ++vertex) {
            if (vertex != from && graph.getMatrixValue(vertex, edge) == 1) {
                return vertex;
            }
        }

        return -1;
    }

    static void printResult(
        const int start,
        const int end,
        const int* distance,
        const int* previous
    ) {
        std::cout << "Dijkstra result:\n";

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

#endif //AIZO_PROJECT2_DIJKSTRA_H
