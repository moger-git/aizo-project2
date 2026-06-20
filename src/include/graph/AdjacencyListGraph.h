#ifndef AIZO_PROJECT2_ADJACENCYLISTGRAPH_H
#define AIZO_PROJECT2_ADJACENCYLISTGRAPH_H

#include <iostream>

struct AdjNode {
    int to;
    int weight;
    AdjNode* next;
};

class AdjacencyListGraph {

    int vertexCount;


    // Liczba logicznych krawędzi grafu
    int edgeCount;

    AdjNode** successors;

    bool areEdgeParametersValid(
        const int from,
        const int to,
        const int weight
    ) const {
        return from >= 0 &&
               from < vertexCount &&
               to >= 0 &&
               to < vertexCount &&
               weight >= 0;
    }

public:
    explicit AdjacencyListGraph(const int vertices)
        : vertexCount(vertices),
          edgeCount(0),
          successors(nullptr) {
        if (vertexCount <= 0) {
            return;
        }

        successors = new AdjNode*[vertexCount];

        for (int i = 0; i < vertexCount; ++i) {
            successors[i] = nullptr;
        }
    }

    ~AdjacencyListGraph() {
        if (successors == nullptr) {
            return;
        }

        for (int i = 0; i < vertexCount; ++i) {
            AdjNode* current = successors[i];

            while (current != nullptr) {
                AdjNode* next = current->next;
                delete current;
                current = next;
            }
        }

        delete[] successors;
    }

    bool addDirectedEdge(
        const int from,
        const int to,
        const int weight
    ) {
        if (!areEdgeParametersValid(from, to, weight)) {
            return false;
        }

        auto* node = new AdjNode{
            to,
            weight,
            successors[from]
        };

        successors[from] = node;

        ++edgeCount;

        return true;
    }

    bool addUndirectedEdge(
        const int from,
        const int to,
        const int weight
    ) {
        if (!areEdgeParametersValid(from, to, weight)) {
            return false;
        }


        //Pierwszy wpis: from -> to
        auto* firstNode = new AdjNode{
            to,
            weight,
            successors[from]
        };

        successors[from] = firstNode;


        //Drugi wpis: to -> from
        auto* secondNode = new AdjNode{
            from,
            weight,
            successors[to]
        };

        successors[to] = secondNode;

        ++edgeCount;

        return true;
    }

    int getVertexCount() const {
        return vertexCount;
    }

    int getEdgeCount() const {
        return edgeCount;
    }

    AdjNode* getNeighbors(const int vertex) const {
        if (
            vertex < 0 ||
            vertex >= vertexCount ||
            successors == nullptr
        ) {
            return nullptr;
        }

        return successors[vertex];
    }

    void print() const {
        std::cout << "Adjacency list:\n";

        for (int i = 0; i < vertexCount; ++i) {
            std::cout << i << ": ";

            const AdjNode* current = successors[i];

            while (current != nullptr) {
                std::cout
                    << "("
                    << current->to
                    << ", w="
                    << current->weight
                    << ") ";

                current = current->next;
            }

            std::cout << "\n";
        }
    }
};

#endif // AIZO_PROJECT2_ADJACENCYLISTGRAPH_H