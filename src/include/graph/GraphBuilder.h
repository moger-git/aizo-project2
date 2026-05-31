#ifndef AIZO_PROJECT2_GRAPHBUILDER_H
#define AIZO_PROJECT2_GRAPHBUILDER_H

#include "graph/AdjacencyListGraph.h"
#include "graph/IncidenceMatrixGraph.h"
#include "io/GraphLoader.h"

class GraphBuilder {
public:
    static void buildAdjacencyListGraph(
        const GraphData& data,
        AdjacencyListGraph& graph,
        const bool undirected
    ) {
        for (int i = 0; i < data.edgeCount; ++i) {
            const int from = data.edges[i].from;
            const int to = data.edges[i].to;
            const int weight = data.edges[i].weight;

            if (undirected) {
                graph.addUndirectedEdge(from, to, weight);
            } else {
                graph.addDirectedEdge(from, to, weight);
            }
        }
    }

    static void buildIncidenceMatrixGraph(
        const GraphData& data,
        IncidenceMatrixGraph& graph,
        const bool undirected
    ) {
        for (int i = 0; i < data.edgeCount; ++i) {
            const int from = data.edges[i].from;
            const int to = data.edges[i].to;
            const int weight = data.edges[i].weight;

            if (undirected) {
                graph.addUndirectedEdge(from, to, weight);
            } else {
                graph.addDirectedEdge(from, to, weight);
            }
        }
    }
};

#endif //AIZO_PROJECT2_GRAPHBUILDER_H
