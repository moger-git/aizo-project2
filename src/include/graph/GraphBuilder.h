#ifndef AIZO_PROJECT2_GRAPHBUILDER_H
#define AIZO_PROJECT2_GRAPHBUILDER_H

#include "graph/AdjacencyListGraph.h"
#include "graph/GraphData.h"
#include "graph/IncidenceMatrixGraph.h"

class GraphBuilder {
public:
    static bool buildAdjacencyListGraph(
        const GraphData& data,
        AdjacencyListGraph& graph,
        const bool undirected
    ) {
        for (int i = 0; i < data.edgeCount; ++i) {
            const int from = data.edges[i].from;
            const int to = data.edges[i].to;
            const int weight = data.edges[i].weight;

            bool edgeAdded = false;

            if (undirected) {
                edgeAdded = graph.addUndirectedEdge(
                    from,
                    to,
                    weight
                );
            } else {
                edgeAdded = graph.addDirectedEdge(
                    from,
                    to,
                    weight
                );
            }

            if (!edgeAdded) {
                return false;
            }
        }

        return true;
    }

    static bool buildIncidenceMatrixGraph(
        const GraphData& data,
        IncidenceMatrixGraph& graph,
        const bool undirected
    ) {
        for (int i = 0; i < data.edgeCount; ++i) {
            const int from = data.edges[i].from;
            const int to = data.edges[i].to;
            const int weight = data.edges[i].weight;

            bool edgeAdded = false;

            if (undirected) {
                edgeAdded = graph.addUndirectedEdge(
                    from,
                    to,
                    weight
                );
            } else {
                edgeAdded = graph.addDirectedEdge(
                    from,
                    to,
                    weight
                );
            }

            if (!edgeAdded) {
                return false;
            }
        }

        return true;
    }
};

#endif // AIZO_PROJECT2_GRAPHBUILDER_H