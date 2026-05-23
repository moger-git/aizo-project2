#ifndef AIZO_PROJECT2_KRUSKAL_H
#define AIZO_PROJECT2_KRUSKAL_H

#include <iostream>

#include "graph/AdjacencyListGraph.h"
#include "graph/IncidenceMatrixGraph.h"
#include "graph/Edge.h"

class Kruskal {
public:
    static void run(
    const AdjacencyListGraph& graph,
    const bool shouldPrintResult = true
    ) {
        const int vertexCount = graph.getVertexCount();

        if (vertexCount <= 0) {
            std::cout << "Kruskal result:\n";
            std::cout << "Graph is empty.\n";
            return;
        }

        int edgeCount = 0;
        Edge* edges = buildEdgesFromAdjacencyList(graph, edgeCount);

        runKruskal(vertexCount, edges, edgeCount, shouldPrintResult);

        delete[] edges;
    }

    static void run(
    const IncidenceMatrixGraph& graph,
    const bool shouldPrintResult = true
    ) {
        const int vertexCount = graph.getVertexCount();

        if (vertexCount <= 0) {
            std::cout << "Kruskal result:\n";
            std::cout << "Graph is empty.\n";
            return;
        }

        int edgeCount = 0;
        Edge* edges = buildEdgesFromIncidenceMatrix(graph, edgeCount);

        runKruskal(vertexCount, edges, edgeCount, shouldPrintResult);

        delete[] edges;
    }

private:
    static Edge* buildEdgesFromAdjacencyList(
        const AdjacencyListGraph& graph,
        int& outputEdgeCount
    ) {
        const int vertexCount = graph.getVertexCount();

        int maxEdges = 0;

        for (int from = 0; from < vertexCount; ++from) {
            AdjNode* current = graph.getNeighbors(from);

            while (current != nullptr) {
                if (from < current->to) {
                    ++maxEdges;
                }

                current = current->next;
            }
        }

        Edge* edges = new Edge[maxEdges];
        outputEdgeCount = 0;

        for (int from = 0; from < vertexCount; ++from) {
            AdjNode* current = graph.getNeighbors(from);

            while (current != nullptr) {
                const int to = current->to;
                const int weight = current->weight;

                if (from < to) {
                    edges[outputEdgeCount].from = from;
                    edges[outputEdgeCount].to = to;
                    edges[outputEdgeCount].weight = weight;
                    ++outputEdgeCount;
                }

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
            int firstVertex = -1;
            int secondVertex = -1;

            for (int vertex = 0; vertex < vertexCount; ++vertex) {
                if (graph.getMatrixValue(vertex, edge) == 1) {
                    if (firstVertex == -1) {
                        firstVertex = vertex;
                    } else {
                        secondVertex = vertex;
                    }
                }
            }

            if (firstVertex != -1 && secondVertex != -1) {
                edges[outputEdgeCount].from = firstVertex;
                edges[outputEdgeCount].to = secondVertex;
                edges[outputEdgeCount].weight = graph.getWeight(edge);
                ++outputEdgeCount;
            }
        }

        return edges;
    }

    static void runKruskal(
        const int vertexCount,
        Edge* edges,
        const int edgeCount,
        const bool shouldPrintResult
    ){
        sortEdgesByWeight(edges, edgeCount);

        int* parent = new int[vertexCount];
        int* rank = new int[vertexCount];

        initializeUnionFind(vertexCount, parent, rank);

        int totalCost = 0;
        int selectedEdges = 0;

        if (shouldPrintResult) {
            std::cout << "Kruskal result:\n";
            std::cout << "Edges in MST:\n";
        }

        for (int i = 0; i < edgeCount && selectedEdges < vertexCount - 1; ++i) {
            const int from = edges[i].from;
            const int to = edges[i].to;
            const int weight = edges[i].weight;

            const int rootFrom = find(parent, from);
            const int rootTo = find(parent, to);

            if (rootFrom != rootTo) {
                unite(parent, rank, rootFrom, rootTo);

                if (shouldPrintResult) {
                    std::cout << from
                              << " - "
                              << to
                              << " weight "
                              << weight
                              << "\n";
                }

                totalCost += weight;
                ++selectedEdges;
            }
        }

        if (selectedEdges != vertexCount - 1 && shouldPrintResult) {
            std::cout << "Warning: graph is not connected. MST does not include all vertices.\n";
        }

        if (shouldPrintResult) {
            std::cout << "MST cost: " << totalCost << "\n";
        }
        delete[] parent;
        delete[] rank;
    }

    static void sortEdgesByWeight(Edge* edges, const int edgeCount) {
        for (int i = 1; i < edgeCount; ++i) {
            Edge key = edges[i];
            int j = i - 1;

            while (j >= 0 && edges[j].weight > key.weight) {
                edges[j + 1] = edges[j];
                --j;
            }

            edges[j + 1] = key;
        }
    }

    static void initializeUnionFind(
        const int vertexCount,
        int* parent,
        int* rank
    ) {
        for (int i = 0; i < vertexCount; ++i) {
            parent[i] = i;
            rank[i] = 0;
        }
    }

    static int find(int* parent, const int vertex) {
        if (parent[vertex] != vertex) {
            parent[vertex] = find(parent, parent[vertex]);
        }

        return parent[vertex];
    }

    static void unite(
        int* parent,
        int* rank,
        const int firstRoot,
        const int secondRoot
    ) {
        if (firstRoot == secondRoot) {
            return;
        }

        if (rank[firstRoot] < rank[secondRoot]) {
            parent[firstRoot] = secondRoot;
        } else if (rank[firstRoot] > rank[secondRoot]) {
            parent[secondRoot] = firstRoot;
        } else {
            parent[secondRoot] = firstRoot;
            ++rank[firstRoot];
        }
    }
};

#endif //AIZO_PROJECT2_KRUSKAL_H
