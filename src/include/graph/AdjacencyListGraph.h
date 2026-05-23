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
    int edgeCount;
    AdjNode** successors;

public:
    explicit AdjacencyListGraph(const int vertices)
        : vertexCount(vertices), edgeCount(0) {
        successors = new AdjNode*[vertexCount];

        for (int i = 0; i < vertexCount; ++i) {
            successors[i] = nullptr;
        }
    }

    ~AdjacencyListGraph() {
        for (int i = 0; i < vertexCount; ++i) {
            const AdjNode* current = successors[i];

            while (current != nullptr) {
                const AdjNode* next = current->next;
                delete current;
                current = next;
            }
        }

        delete[] successors;
    }

    void addDirectedEdge(const int from, const int to, const int weight) {
        auto* node = new AdjNode{to, weight, successors[from]};
        successors[from] = node;
        ++edgeCount;
    }

    void addUndirectedEdge(const int from, const int to, const int weight) {
        addDirectedEdge(from, to, weight);
        addDirectedEdge(to, from, weight);
    }

    int getVertexCount() const {
        return vertexCount;
    }

    AdjNode* getNeighbors(const int vertex) const {
        return successors[vertex];
    }

    void print() const {
        for (int i = 0; i < vertexCount; ++i) {
            std::cout << i << ": ";

            const AdjNode* current = successors[i];
            while (current != nullptr) {
                std::cout << "(" << current->to << ", w=" << current->weight << ") ";
                current = current->next;
            }

            std::cout << "\n";
        }
    }
};

#endif //AIZO_PROJECT2_ADJACENCYLISTGRAPH_H
