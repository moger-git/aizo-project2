#include <climits>
#include <cstdlib>
#include <iostream>
#include <string>

#include "lib/Parameters.h"
#include "io/GraphLoader.h"
#include "io/CsvLogger.h"
#include "graph/AdjacencyListGraph.h"
#include "graph/IncidenceMatrixGraph.h"
#include "graph/GraphBuilder.h"
#include "algorithms/Dijkstra.h"
#include "algorithms/Prim.h"
#include "algorithms/Kruskal.h"
#include "algorithms/BellmanFord.h"
#include "algorithms/FordFulkerson.h"
#include "graph/GraphGenerator.h"
#include "utils/Timer.h"
#include "io/DateTime.h"

template <typename Graph>
void runSelectedAlgorithm(const Graph& graph) {
    std::cout << "\nSolution:\n";

    if (Parameters::problem == Parameters::Problems::mst) {
        if (Parameters::algorithm == Parameters::Algorithms::prim) {
            std::cout << "Selected algorithm: Prim\n";

            Prim::run(graph);

            return;
        }

        if (Parameters::algorithm == Parameters::Algorithms::kruskal) {
            std::cout << "Selected algorithm: Kruskal\n";

            Kruskal::run(graph);

            return;
        }

        std::cout << "Error: unsupported MST algorithm.\n";
        return;
    }

    if (Parameters::problem == Parameters::Problems::sp) {
        if (Parameters::algorithm == Parameters::Algorithms::dijkstra) {
            std::cout << "Selected algorithm: Dijkstra\n";

            Dijkstra::run(
                graph,
                Parameters::vertexStart,
                Parameters::vertexEnd
            );

            return;
        }

        if (Parameters::algorithm == Parameters::Algorithms::bellmanFord) {
            std::cout << "Selected algorithm: Bellman-Ford\n";

            BellmanFord::run(
                graph,
                Parameters::vertexStart,
                Parameters::vertexEnd
            );

            return;
        }

        std::cout << "Error: unsupported SP algorithm.\n";
        return;
    }

    if (Parameters::problem == Parameters::Problems::mf) {
        if (Parameters::algorithm == Parameters::Algorithms::fordFulkerson) {
            std::cout << "Selected algorithm: Ford-Fulkerson\n";

            FordFulkerson::run(
                graph,
                Parameters::vertexStart,
                Parameters::vertexEnd
            );

            return;
        }

        std::cout << "Error: unsupported MF algorithm.\n";
        return;
    }

    std::cout << "Error: unsupported problem.\n";
}

std::string getProblemName(const Parameters::Problems problem) {
    if (problem == Parameters::Problems::mst) {
        return "MST";
    }

    if (problem == Parameters::Problems::sp) {
        return "SP";
    }

    if (problem == Parameters::Problems::mf) {
        return "MF";
    }

    return "unknown";
}

std::string getAlgorithmName(const Parameters::Algorithms algorithm) {
    if (algorithm == Parameters::Algorithms::prim) {
        return "Prim";
    }

    if (algorithm == Parameters::Algorithms::kruskal) {
        return "Kruskal";
    }

    if (algorithm == Parameters::Algorithms::dijkstra) {
        return "Dijkstra";
    }

    if (algorithm == Parameters::Algorithms::bellmanFord) {
        return "Bellman-Ford";
    }

    if (algorithm == Parameters::Algorithms::fordFulkerson) {
        return "Ford-Fulkerson";
    }

    return "unknown";
}

std::string getStructureName(const Parameters::Structures structure) {
    if (structure == Parameters::Structures::incidenceMatrix) {
        return "IncidenceMatrix";
    }

    if (structure == Parameters::Structures::adjacencyList) {
        return "AdjacencyList";
    }

    return "unknown";
}

template <typename Graph>
long long measureAlgorithm(
    const Graph& graph,
    const Parameters::Problems problem,
    const Parameters::Algorithms algorithm,
    const int startVertex,
    const int endVertex
) {
    Timer timer;

    timer.start();

    if (problem == Parameters::Problems::mst) {
        if (algorithm == Parameters::Algorithms::prim) {
            Prim::run(graph, false);
        } else if (algorithm == Parameters::Algorithms::kruskal) {
            Kruskal::run(graph, false);
        }
    } else if (problem == Parameters::Problems::sp) {
        if (algorithm == Parameters::Algorithms::dijkstra) {
            Dijkstra::run(graph, startVertex, endVertex, false);
        } else if (algorithm == Parameters::Algorithms::bellmanFord) {
            BellmanFord::run(graph, startVertex, endVertex, false);
        }
    } else if (problem == Parameters::Problems::mf) {
        if (algorithm == Parameters::Algorithms::fordFulkerson) {
            FordFulkerson::run(graph, startVertex, endVertex, false);
        }
    }

    return timer.stopMicroseconds();
}

int runSingleFileMode() {
    GraphData data;

    if (!GraphLoader::load(Parameters::inputFile, data)) {
        std::cout << "Error: cannot load input graph.\n";
        return 1;
    }

    bool undirected = Parameters::problem == Parameters::Problems::mst;

    std::cout << "Loaded graph:\n";
    std::cout << "Vertices: " << data.vertexCount << "\n";
    std::cout << "Edges: " << data.edgeCount << "\n";

    if (Parameters::structure == Parameters::Structures::adjacencyList) {
        AdjacencyListGraph graph(data.vertexCount);

        GraphBuilder::buildAdjacencyListGraph(data, graph, undirected);

        std::cout << "\nGraph representation: adjacency list\n";
        graph.print();

        runSelectedAlgorithm(graph);
    } else if (Parameters::structure == Parameters::Structures::incidenceMatrix) {
        IncidenceMatrixGraph graph(data.vertexCount, data.edgeCount);

        GraphBuilder::buildIncidenceMatrixGraph(data, graph, undirected);

        std::cout << "\nGraph representation: incidence matrix\n";
        graph.print();

        runSelectedAlgorithm(graph);
    } else {
        std::cout << "Error: unsupported graph representation.\n";
        GraphLoader::free(data);
        return 1;
    }

    GraphLoader::free(data);
    return 0;
}

void runBenchmarkCase(
    const Parameters::Problems problem,
    const Parameters::Algorithms algorithm,
    const Parameters::Structures structure,
    const int vertexCount,
    const int density,
    const int iterations,
    const std::string& outputFile
) {
    const bool directed = problem != Parameters::Problems::mst;
    const bool undirected = problem == Parameters::Problems::mst;

    const int startVertex = 0;
    const int endVertex = vertexCount - 1;

    long long minTime = LLONG_MAX;
    long long maxTime = 0;
    long long sumTime = 0;
    int lastEdgeCount = 0;

    for (int iteration = 0; iteration < iterations; ++iteration) {
        GraphData data = GraphGenerator::generate(
            vertexCount,
            density,
            directed
        );

        lastEdgeCount = data.edgeCount;

        long long timeUs = 0;

        if (structure == Parameters::Structures::adjacencyList) {
            AdjacencyListGraph graph(data.vertexCount);

            GraphBuilder::buildAdjacencyListGraph(
                data,
                graph,
                undirected
            );

            timeUs = measureAlgorithm(
                graph,
                problem,
                algorithm,
                startVertex,
                endVertex
            );
        } else if (structure == Parameters::Structures::incidenceMatrix) {
            IncidenceMatrixGraph graph(data.vertexCount, data.edgeCount);

            GraphBuilder::buildIncidenceMatrixGraph(
                data,
                graph,
                undirected
            );

            timeUs = measureAlgorithm(
                graph,
                problem,
                algorithm,
                startVertex,
                endVertex
            );
        }

        if (timeUs < minTime) {
            minTime = timeUs;
        }

        if (timeUs > maxTime) {
            maxTime = timeUs;
        }

        sumTime += timeUs;

        GraphLoader::free(data);
    }

    const double avgTime = static_cast<double>(sumTime) / iterations;

    BenchmarkResult result;

    result.date = getCurrentDateTime();
    result.problem = getProblemName(problem);
    result.algorithm = getAlgorithmName(algorithm);
    result.structure = getStructureName(structure);
    result.vertexCount = vertexCount;
    result.edgeCount = lastEdgeCount;
    result.density = density;
    result.iterations = iterations;
    result.minTimeUs = minTime;
    result.maxTimeUs = maxTime;
    result.avgTimeUs = avgTime;

    CsvLogger::appendResult(outputFile, result);

    std::cout << "Saved benchmark: "
              << result.problem << "; "
              << result.algorithm << "; "
              << result.structure << "; V="
              << result.vertexCount << "; density="
              << result.density << "%; avg="
              << result.avgTimeUs << " us\n";
}

void runAlgorithmsForProblem(
    const Parameters::Problems problem,
    const Parameters::Structures structure,
    const int vertexCount,
    const int density,
    const int iterations,
    const std::string& outputFile
) {
    if (problem == Parameters::Problems::mst) {
        runBenchmarkCase(
            problem,
            Parameters::Algorithms::prim,
            structure,
            vertexCount,
            density,
            iterations,
            outputFile
        );

        runBenchmarkCase(
            problem,
            Parameters::Algorithms::kruskal,
            structure,
            vertexCount,
            density,
            iterations,
            outputFile
        );
    } else if (problem == Parameters::Problems::sp) {
        runBenchmarkCase(
            problem,
            Parameters::Algorithms::dijkstra,
            structure,
            vertexCount,
            density,
            iterations,
            outputFile
        );

        runBenchmarkCase(
            problem,
            Parameters::Algorithms::bellmanFord,
            structure,
            vertexCount,
            density,
            iterations,
            outputFile
        );
    } else if (problem == Parameters::Problems::mf) {
        runBenchmarkCase(
            problem,
            Parameters::Algorithms::fordFulkerson,
            structure,
            vertexCount,
            density,
            iterations,
            outputFile
        );
    }
}

int runBenchmarkMode() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    std::cout << "Benchmark mode selected.\n";
    std::cout << "Problem: " << getProblemName(Parameters::problem) << "\n";
    std::cout << "Algorithm: " << getAlgorithmName(Parameters::algorithm) << "\n";
    std::cout << "Structure: " << getStructureName(Parameters::structure) << "\n";
    std::cout << "VertexCount: " << Parameters::vertexCount << "\n";
    std::cout << "Density: " << Parameters::density << "%\n";
    std::cout << "Iterations: " << Parameters::iterations << "\n";
    std::cout << "Results file: " << Parameters::resultsFile << "\n";

    if (Parameters::algorithm == Parameters::Algorithms::allAlgorithms) {
        runAlgorithmsForProblem(
            Parameters::problem,
            Parameters::structure,
            Parameters::vertexCount,
            Parameters::density,
            Parameters::iterations,
            Parameters::resultsFile
        );

        return 0;
    }

    if (Parameters::structure == Parameters::Structures::allStructures) {
        runBenchmarkCase(
            Parameters::problem,
            Parameters::algorithm,
            Parameters::Structures::incidenceMatrix,
            Parameters::vertexCount,
            Parameters::density,
            Parameters::iterations,
            Parameters::resultsFile
        );

        runBenchmarkCase(
            Parameters::problem,
            Parameters::algorithm,
            Parameters::Structures::adjacencyList,
            Parameters::vertexCount,
            Parameters::density,
            Parameters::iterations,
            Parameters::resultsFile
        );

        return 0;
    }

    runBenchmarkCase(
        Parameters::problem,
        Parameters::algorithm,
        Parameters::structure,
        Parameters::vertexCount,
        Parameters::density,
        Parameters::iterations,
        Parameters::resultsFile
    );

    return 0;
}

int main(int argc, char** argv) {
    int result = Parameters::readParameters(argc - 1, argv + 1);

    if (result != 0) {
        std::cout << "Error: invalid parameters.\n";
        Parameters::help();
        return result;
    }

    if (Parameters::runMode == Parameters::RunModes::help) {
        Parameters::help();
        return 0;
    }

    if (Parameters::runMode == Parameters::RunModes::singleFile) {
        return runSingleFileMode();
    }

    if (Parameters::runMode == Parameters::RunModes::benchmark) {
        return runBenchmarkMode();
    }

    std::cout << "Error: unknown run mode.\n";
    return 1;
}