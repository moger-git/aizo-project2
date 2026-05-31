#ifndef AIZO_PROJECT2_GRAPHDATA_H
#define AIZO_PROJECT2_GRAPHDATA_H

#include "graph/Edge.h"

struct GraphData {
    int edgeCount{};
    int vertexCount{};
    Edge* edges{};
};

#endif //AIZO_PROJECT2_GRAPHDATA_H
