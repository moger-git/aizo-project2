#ifndef AIZO_PROJECT2_GRAPH_GENERATOR_H
#define AIZO_PROJECT2_GRAPH_GENERATOR_H

#include <cstdlib>
#include <ctime>

#include "graph/GraphData.h"
#include "graph/Edge.h"

class GraphGenerator {
public:
    static GraphData generate(
        const int vertexCount,
        const int density,
        const bool directed
    ) {
        GraphData data{};

        if (vertexCount <= 0) {
            data.vertexCount = 0;
            data.edgeCount = 0;
            data.edges = nullptr;
            return data;
        }

        const int maxEdges = calculateMaxEdges(vertexCount, directed);
        int targetEdges = maxEdges * density / 100;

        if (targetEdges < vertexCount - 1) {
            targetEdges = vertexCount - 1;
        }

        if (targetEdges > maxEdges) {
            targetEdges = maxEdges;
        }

        data.vertexCount = vertexCount;
        data.edgeCount = targetEdges;
        data.edges = new Edge[targetEdges];

        bool** exists = createExistMatrix(vertexCount);

        int edgeIndex = 0;

        generateSpanningTree(
            data,
            exists,
            edgeIndex,
            vertexCount,
            directed,
            targetEdges
        );

        generateRemainingEdges(
            data,
            exists,
            edgeIndex,
            vertexCount,
            directed,
            targetEdges
        );

        deleteExistMatrix(exists, vertexCount);

        return data;
    }

private:
    static int calculateMaxEdges(const int vertexCount, const bool directed) {
        if (directed) {
            return vertexCount * (vertexCount - 1);
        }

        return vertexCount * (vertexCount - 1) / 2;
    }

    static bool** createExistMatrix(const int vertexCount) {
        bool** matrix = new bool*[vertexCount];

        for (int i = 0; i < vertexCount; ++i) {
            matrix[i] = new bool[vertexCount];

            for (int j = 0; j < vertexCount; ++j) {
                matrix[i][j] = false;
            }
        }

        return matrix;
    }

    static void deleteExistMatrix(bool** matrix, const int vertexCount) {
        for (int i = 0; i < vertexCount; ++i) {
            delete[] matrix[i];
        }

        delete[] matrix;
    }

    static int randomWeight(const int edgeCount) {
        int maxWeight = edgeCount * 4 / 5;

        if (maxWeight < 1) {
            maxWeight = 1;
        }

        return 1 + std::rand() % maxWeight;
    }

    static void markEdge(
        bool** exists,
        const int from,
        const int to,
        const bool directed
    ) {
        exists[from][to] = true;

        if (!directed) {
            exists[to][from] = true;
        }
    }

    static bool edgeExists(
        bool** exists,
        const int from,
        const int to
    ) {
        return exists[from][to];
    }

    static void addEdge(
        GraphData& data,
        bool** exists,
        int& edgeIndex,
        const int from,
        const int to,
        const int weight,
        const bool directed
    ) {
        data.edges[edgeIndex].from = from;
        data.edges[edgeIndex].to = to;
        data.edges[edgeIndex].weight = weight;

        ++edgeIndex;

        markEdge(exists, from, to, directed);
    }

    static void generateSpanningTree(
        GraphData& data,
        bool** exists,
        int& edgeIndex,
        const int vertexCount,
        const bool directed,
        const int targetEdges
    ) {
        for (int vertex = 1; vertex < vertexCount && edgeIndex < targetEdges; ++vertex) {
            const int parent = std::rand() % vertex;
            const int weight = randomWeight(targetEdges);

            addEdge(
                data,
                exists,
                edgeIndex,
                parent,
                vertex,
                weight,
                directed
            );
        }
    }

    static void generateRemainingEdges(
        GraphData& data,
        bool** exists,
        int& edgeIndex,
        const int vertexCount,
        const bool directed,
        const int targetEdges
    ) {
        while (edgeIndex < targetEdges) {
            const int from = std::rand() % vertexCount;
            const int to = std::rand() % vertexCount;

            if (from == to) {
                continue;
            }

            if (edgeExists(exists, from, to)) {
                continue;
            }

            const int weight = randomWeight(targetEdges);

            addEdge(
                data,
                exists,
                edgeIndex,
                from,
                to,
                weight,
                directed
            );
        }
    }
};

#endif //AIZO_PROJECT2_GRAPH_GENERATOR_H
