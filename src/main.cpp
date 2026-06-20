#include <climits>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <streambuf>
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
bool runSingleAlgorithm(
    const Graph& graph,
    const Parameters::Problems problem,
    const Parameters::Algorithms algorithm
) {
    std::cout << "\nSolution:\n";

    if (problem == Parameters::Problems::mst) {
        if (algorithm == Parameters::Algorithms::prim) {
            std::cout << "Selected algorithm: Prim\n";
            Prim::run(graph);
            return true;
        }

        if (algorithm == Parameters::Algorithms::kruskal) {
            std::cout << "Selected algorithm: Kruskal\n";
            Kruskal::run(graph);
            return true;
        }

        std::cout << "Error: unsupported MST algorithm.\n";
        return false;
    }

    if (problem == Parameters::Problems::sp) {
        if (algorithm == Parameters::Algorithms::dijkstra) {
            std::cout << "Selected algorithm: Dijkstra\n";

            Dijkstra::run(
                graph,
                Parameters::vertexStart,
                Parameters::vertexEnd
            );

            return true;
        }

        if (algorithm == Parameters::Algorithms::bellmanFord) {
            std::cout << "Selected algorithm: Bellman-Ford\n";

            BellmanFord::run(
                graph,
                Parameters::vertexStart,
                Parameters::vertexEnd
            );

            return true;
        }

        std::cout << "Error: unsupported SP algorithm.\n";
        return false;
    }

    if (problem == Parameters::Problems::mf) {
        if (algorithm == Parameters::Algorithms::fordFulkerson) {
            std::cout << "Selected algorithm: Ford-Fulkerson\n";

            FordFulkerson::run(
                graph,
                Parameters::vertexStart,
                Parameters::vertexEnd
            );

            return true;
        }

        std::cout << "Error: unsupported MF algorithm.\n";
        return false;
    }

    std::cout << "Error: unsupported problem.\n";
    return false;
}

template <typename Graph>
bool runAlgorithmsForSingleFileGraph(const Graph& graph) {

    if (
        Parameters::algorithm !=
        Parameters::Algorithms::allAlgorithms
    ) {
        return runSingleAlgorithm(
            graph,
            Parameters::problem,
            Parameters::algorithm
        );
    }

    if (Parameters::problem == Parameters::Problems::mst) {
        bool success = true;

        if (!runSingleAlgorithm(
                graph,
                Parameters::problem,
                Parameters::Algorithms::prim
            )) {
            success = false;
            }

        if (!runSingleAlgorithm(
                graph,
                Parameters::problem,
                Parameters::Algorithms::kruskal
            )) {
            success = false;
            }

        return success;
    }

    if (Parameters::problem == Parameters::Problems::sp) {
        bool success = true;

        if (!runSingleAlgorithm(
                graph,
                Parameters::problem,
                Parameters::Algorithms::dijkstra
            )) {
            success = false;
            }

        if (!runSingleAlgorithm(
                graph,
                Parameters::problem,
                Parameters::Algorithms::bellmanFord
            )) {
            success = false;
            }

        return success;
    }

    if (Parameters::problem == Parameters::Problems::mf) {
        return runSingleAlgorithm(
            graph,
            Parameters::problem,
            Parameters::Algorithms::fordFulkerson
        );
    }

    std::cout << "Error: unsupported problem.\n";
    return false;
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
    if (algorithm == Parameters::Algorithms::allAlgorithms) {
        return "AllAlgorithms";
    }

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
    if (structure == Parameters::Structures::allStructures) {
        return "AllStructures";
    }

    if (structure == Parameters::Structures::incidenceMatrix) {
        return "IncidenceMatrix";
    }

    if (structure == Parameters::Structures::adjacencyList) {
        return "AdjacencyList";
    }

    return "unknown";
}

bool isAlgorithmCompatible(
    const Parameters::Problems problem,
    const Parameters::Algorithms algorithm
) {

    if (algorithm == Parameters::Algorithms::allAlgorithms) {
        return true;
    }

    if (problem == Parameters::Problems::mst) {
        return algorithm == Parameters::Algorithms::prim ||
               algorithm == Parameters::Algorithms::kruskal;
    }

    if (problem == Parameters::Problems::sp) {
        return algorithm == Parameters::Algorithms::dijkstra ||
               algorithm == Parameters::Algorithms::bellmanFord;
    }

    if (problem == Parameters::Problems::mf) {
        return algorithm == Parameters::Algorithms::fordFulkerson;
    }

    return false;
}

bool validateBenchmarkParameters() {
    bool valid = true;

    if (
        Parameters::problem == Parameters::Problems::undefined ||
        Parameters::problem == Parameters::Problems::count
    ) {
        std::cout << "Error: problem is not defined correctly.\n";
        valid = false;
    }

    if (
        Parameters::algorithm == Parameters::Algorithms::undefined ||
        Parameters::algorithm == Parameters::Algorithms::count
    ) {
        std::cout << "Error: algorithm is not defined correctly.\n";
        valid = false;
    }

    if (
        Parameters::structure == Parameters::Structures::undefined ||
        Parameters::structure == Parameters::Structures::count
    ) {
        std::cout << "Error: graph structure is not defined correctly.\n";
        valid = false;
    }

    if (Parameters::vertexCount < 2) {
        std::cout << "Error: vertexCount must be at least 2.\n";
        valid = false;
    }

    if (Parameters::density < 1 || Parameters::density > 100) {
        std::cout << "Error: density must be in range 1-100.\n";
        valid = false;
    }

    if (Parameters::iterations <= 0) {
        std::cout << "Error: iterations must be greater than 0.\n";
        valid = false;
    }

    if (Parameters::resultsFile.empty()) {
        std::cout << "Error: results file path is empty.\n";
        valid = false;
    }

    if (
        Parameters::problem != Parameters::Problems::undefined &&
        Parameters::algorithm != Parameters::Algorithms::undefined &&
        !isAlgorithmCompatible(
            Parameters::problem,
            Parameters::algorithm
        )
    ) {
        std::cout
            << "Error: algorithm "
            << getAlgorithmName(Parameters::algorithm)
            << " cannot solve problem "
            << getProblemName(Parameters::problem)
            << ".\n";

        valid = false;
    }

    return valid;
}
// Tworzy powtarzalne ziarno generatora grafu
unsigned int createBenchmarkSeed(
    const Parameters::Problems problem,
    const int vertexCount,
    const int density,
    const int iteration
) {
    unsigned int seed = 2166136261u;

    seed =
        (seed ^ static_cast<unsigned int>(problem))
        * 16777619u;

    seed =
        (seed ^ static_cast<unsigned int>(vertexCount))
        * 16777619u;

    seed =
        (seed ^ static_cast<unsigned int>(density))
        * 16777619u;

    seed =
        (seed ^ static_cast<unsigned int>(iteration))
        * 16777619u;

    return seed;
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

double calculateGraphDensity(
    const GraphData& data,
    const bool directed
) {
    if (data.vertexCount <= 1) {
        return 0.0;
    }

    const long long vertices = data.vertexCount;

    long long maxEdgeCount = 0;

    if (directed) {
        maxEdgeCount = vertices * (vertices - 1);
    } else {
        maxEdgeCount =
            vertices * (vertices - 1) / 2;
    }

    if (maxEdgeCount == 0) {
        return 0.0;
    }

    return (
        static_cast<double>(data.edgeCount) /
        static_cast<double>(maxEdgeCount)
    ) * 100.0;
}

template <typename Graph>
bool runLoadedGraphBenchmarkCase(
    const Graph& graph,
    const GraphData& data,
    const Parameters::Problems problem,
    const Parameters::Algorithms algorithm,
    const Parameters::Structures structure,
    const int iterations,
    const std::string& resultsFile
) {
    if (iterations <= 0) {
        std::cout
            << "Error: dataset benchmark iterations "
            << "must be greater than 0.\n";

        return false;
    }

    if (
        algorithm ==
        Parameters::Algorithms::allAlgorithms
    ) {
        std::cout
            << "Error: runLoadedGraphBenchmarkCase "
            << "requires one concrete algorithm.\n";

        return false;
    }

    if (!isAlgorithmCompatible(problem, algorithm)) {
        std::cout
            << "Error: algorithm "
            << getAlgorithmName(algorithm)
            << " cannot solve problem "
            << getProblemName(problem)
            << ".\n";

        return false;
    }

    const bool directed =
        problem != Parameters::Problems::mst;

    const int startVertex =
        problem == Parameters::Problems::mst
            ? 0
            : Parameters::vertexStart;

    const int endVertex =
        problem == Parameters::Problems::mst
            ? data.vertexCount - 1
            : Parameters::vertexEnd;

    long long minTime = LLONG_MAX;
    long long maxTime = 0;
    long long sumTime = 0;

    for (int iteration = 0;
         iteration < iterations;
         ++iteration) {
        const long long timeUs = measureAlgorithm(
            graph,
            problem,
            algorithm,
            startVertex,
            endVertex
        );

        if (timeUs < minTime) {
            minTime = timeUs;
        }

        if (timeUs > maxTime) {
            maxTime = timeUs;
        }

        sumTime += timeUs;
    }

    const double avgTime =
        static_cast<double>(sumTime) /
        static_cast<double>(iterations);

    BenchmarkResult result;

    result.date = getCurrentDateTime();
    result.problem = getProblemName(problem);
    result.algorithm = getAlgorithmName(algorithm);
    result.structure = getStructureName(structure);
    result.vertexCount = data.vertexCount;
    result.edgeCount = data.edgeCount;
    result.density = calculateGraphDensity(
        data,
        directed
    );
    result.iterations = iterations;
    result.minTimeUs = minTime;
    result.maxTimeUs = maxTime;
    result.avgTimeUs = avgTime;

    if (!CsvLogger::appendResult(
            resultsFile,
            result
        )) {
        std::cout
            << "Error: dataset benchmark result "
            << "was not saved.\n";

        return false;
    }

    std::cout
        << "Saved dataset benchmark: "
        << result.problem
        << "; "
        << result.algorithm
        << "; "
        << result.structure
        << "; V="
        << result.vertexCount
        << "; E="
        << result.edgeCount
        << "; density="
        << result.density
        << "%; min="
        << result.minTimeUs
        << " us; max="
        << result.maxTimeUs
        << " us; avg="
        << result.avgTimeUs
        << " us\n";

    return true;
}

template <typename Graph>
bool runLoadedGraphAlgorithms(
    const Graph& graph,
    const GraphData& data,
    const Parameters::Structures structure
) {

    if (
        Parameters::algorithm !=
        Parameters::Algorithms::allAlgorithms
    ) {
        return runLoadedGraphBenchmarkCase(
            graph,
            data,
            Parameters::problem,
            Parameters::algorithm,
            structure,
            Parameters::iterations,
            Parameters::resultsFile
        );
    }

    bool success = true;

    if (Parameters::problem == Parameters::Problems::mst) {
        if (!runLoadedGraphBenchmarkCase(
                graph,
                data,
                Parameters::problem,
                Parameters::Algorithms::prim,
                structure,
                Parameters::iterations,
                Parameters::resultsFile
            )) {
            success = false;
        }

        if (!runLoadedGraphBenchmarkCase(
                graph,
                data,
                Parameters::problem,
                Parameters::Algorithms::kruskal,
                structure,
                Parameters::iterations,
                Parameters::resultsFile
            )) {
            success = false;
        }

        return success;
    }

    if (Parameters::problem == Parameters::Problems::sp) {
        if (!runLoadedGraphBenchmarkCase(
                graph,
                data,
                Parameters::problem,
                Parameters::Algorithms::dijkstra,
                structure,
                Parameters::iterations,
                Parameters::resultsFile
            )) {
            success = false;
        }

        if (!runLoadedGraphBenchmarkCase(
                graph,
                data,
                Parameters::problem,
                Parameters::Algorithms::bellmanFord,
                structure,
                Parameters::iterations,
                Parameters::resultsFile
            )) {
            success = false;
        }

        return success;
    }

    if (Parameters::problem == Parameters::Problems::mf) {
        return runLoadedGraphBenchmarkCase(
            graph,
            data,
            Parameters::problem,
            Parameters::Algorithms::fordFulkerson,
            structure,
            Parameters::iterations,
            Parameters::resultsFile
        );
    }

    std::cout << "Error: unsupported dataset problem.\n";
    return false;
}

bool runLoadedGraphForStructure(
    const GraphData& data,
    const Parameters::Structures structure
) {
    const bool undirected =
        Parameters::problem == Parameters::Problems::mst;

    if (
        structure ==
        Parameters::Structures::adjacencyList
    ) {
        AdjacencyListGraph graph(data.vertexCount);

        if (!GraphBuilder::buildAdjacencyListGraph(
                data,
                graph,
                undirected
            )) {
            std::cout
                << "Error: cannot build adjacency list "
                << "for dataset benchmark.\n";

            return false;
        }

        if (
            graph.getVertexCount() != data.vertexCount ||
            graph.getEdgeCount() != data.edgeCount
        ) {
            std::cout
                << "Error: adjacency list verification "
                << "failed for dataset benchmark.\n";

            return false;
        }

        return runLoadedGraphAlgorithms(
            graph,
            data,
            structure
        );
    }

    if (
        structure ==
        Parameters::Structures::incidenceMatrix
    ) {
        IncidenceMatrixGraph graph(
            data.vertexCount,
            data.edgeCount
        );

        if (!GraphBuilder::buildIncidenceMatrixGraph(
                data,
                graph,
                undirected
            )) {
            std::cout
                << "Error: cannot build incidence matrix "
                << "for dataset benchmark.\n";

            return false;
        }

        if (
            graph.getVertexCount() != data.vertexCount ||
            graph.getEdgeCount() != data.edgeCount
        ) {
            std::cout
                << "Error: incidence matrix verification "
                << "failed for dataset benchmark.\n";

            return false;
        }

        return runLoadedGraphAlgorithms(
            graph,
            data,
            structure
        );
    }

    std::cout
        << "Error: unsupported structure "
        << "for dataset benchmark.\n";

    return false;
}

bool runLoadedGraphBenchmark(
    const GraphData& data
) {
    if (Parameters::resultsFile.empty()) {
        std::cout
            << "Error: results file for dataset "
            << "benchmark is empty.\n";

        return false;
    }

    if (Parameters::iterations <= 0) {
        std::cout
            << "Error: dataset benchmark iterations "
            << "must be greater than 0.\n";

        return false;
    }

    std::cout << "Dataset benchmark mode selected.\n";
    std::cout << "Input file: "
              << Parameters::inputFile
              << "\n";
    std::cout << "Problem: "
              << getProblemName(Parameters::problem)
              << "\n";
    std::cout << "Algorithm: "
              << getAlgorithmName(Parameters::algorithm)
              << "\n";
    std::cout << "Structure: "
              << getStructureName(Parameters::structure)
              << "\n";
    std::cout << "Vertices: "
              << data.vertexCount
              << "\n";
    std::cout << "Edges: "
              << data.edgeCount
              << "\n";
    std::cout << "Iterations: "
              << Parameters::iterations
              << "\n";
    std::cout << "Results file: "
              << Parameters::resultsFile
              << "\n";

    if (
        Parameters::structure ==
        Parameters::Structures::allStructures
    ) {
        bool success = true;

        if (!runLoadedGraphForStructure(
                data,
                Parameters::Structures::incidenceMatrix
            )) {
            success = false;
            }

        if (!runLoadedGraphForStructure(
                data,
                Parameters::Structures::adjacencyList
            )) {
            success = false;
            }

        return success;
    }

    return runLoadedGraphForStructure(
        data,
        Parameters::structure
    );
}

bool validateSingleFileParameters(const GraphData& data) {
    bool valid = true;

    if (
        Parameters::problem == Parameters::Problems::undefined ||
        Parameters::problem == Parameters::Problems::count
    ) {
        std::cout << "Error: problem is not defined correctly.\n";
        valid = false;
    }

    if (
        Parameters::algorithm == Parameters::Algorithms::undefined ||
        Parameters::algorithm == Parameters::Algorithms::count
    ) {
        std::cout << "Error: algorithm is not defined correctly.\n";
        valid = false;
    }

    if (
        Parameters::structure == Parameters::Structures::undefined ||
        Parameters::structure == Parameters::Structures::count
    ) {
        std::cout << "Error: graph structure is not defined correctly.\n";
        valid = false;
    }

    if (
        Parameters::problem != Parameters::Problems::undefined &&
        Parameters::algorithm != Parameters::Algorithms::undefined &&
        !isAlgorithmCompatible(
            Parameters::problem,
            Parameters::algorithm
        )
    ) {
        std::cout
            << "Error: algorithm "
            << getAlgorithmName(Parameters::algorithm)
            << " cannot solve problem "
            << getProblemName(Parameters::problem)
            << ".\n";

        valid = false;
    }

    if (Parameters::problem == Parameters::Problems::mst) {
        return valid;
    }

    if (
        Parameters::vertexStart < 0 ||
        Parameters::vertexStart >= data.vertexCount
    ) {
        std::cout
            << "Error: start vertex must be in range 0-"
            << data.vertexCount - 1
            << ".\n";

        valid = false;
    }

    if (
        Parameters::vertexEnd < 0 ||
        Parameters::vertexEnd >= data.vertexCount
    ) {
        std::cout
            << "Error: end vertex must be in range 0-"
            << data.vertexCount - 1
            << ".\n";

        valid = false;
    }

    if (
        Parameters::problem == Parameters::Problems::mf &&
        Parameters::vertexStart == Parameters::vertexEnd
    ) {
        std::cout
            << "Error: source and sink must be different "
            << "for maximum flow.\n";

        valid = false;
    }

    return valid;
}

template <typename Graph>
bool verifyBuiltGraph(
    const GraphData& data,
    const Graph& graph
) {
    if (graph.getVertexCount() != data.vertexCount) {
        std::cout
            << "Error: graph vertex count is incorrect.\n"
            << "Expected: "
            << data.vertexCount
            << "\n"
            << "Created: "
            << graph.getVertexCount()
            << "\n";

        return false;
    }

    if (graph.getEdgeCount() != data.edgeCount) {
        std::cout
            << "Error: graph edge count is incorrect.\n"
            << "Expected: "
            << data.edgeCount
            << "\n"
            << "Created: "
            << graph.getEdgeCount()
            << "\n";

        return false;
    }

    std::cout
        << "Graph verification: correct.\n"
        << "Verified vertices: "
        << graph.getVertexCount()
        << "\n"
        << "Verified edges: "
        << graph.getEdgeCount()
        << "\n";

    return true;
}

bool runSingleFileForStructure(
    const GraphData& data,
    const Parameters::Structures structure
) {
    const bool undirected =
        Parameters::problem == Parameters::Problems::mst;

    if (structure == Parameters::Structures::adjacencyList) {
        AdjacencyListGraph graph(data.vertexCount);

        if (!GraphBuilder::buildAdjacencyListGraph(
                data,
                graph,
                undirected
            )) {
            std::cout
                << "Error: cannot build adjacency list graph.\n";

            return false;
            }

        if (!verifyBuiltGraph(data, graph)) {
            return false;
        }

        std::cout
            << "Graph representation: adjacency list\n";

        graph.print();

        return runAlgorithmsForSingleFileGraph(graph);
    }

    if (structure == Parameters::Structures::incidenceMatrix) {
        IncidenceMatrixGraph graph(
            data.vertexCount,
            data.edgeCount
        );

        if (!GraphBuilder::buildIncidenceMatrixGraph(
                data,
                graph,
                undirected
            )) {
            std::cout
                << "Error: cannot build incidence matrix graph.\n";

            return false;
            }

        if (!verifyBuiltGraph(data, graph)) {
            return false;
        }

        std::cout
            << "Graph representation: incidence matrix\n";

        graph.print();

        return runAlgorithmsForSingleFileGraph(graph);
    }

    std::cout << "Error: unsupported graph representation.\n";
    return false;
}
class CoutRedirect {
private:
    std::streambuf* originalBuffer;
    std::ofstream outputFile;

    bool redirectRequested;
    bool redirectSuccessful;

public:
    explicit CoutRedirect(const std::string& filePath)
        : originalBuffer(nullptr),
          redirectRequested(!filePath.empty()),
          redirectSuccessful(false) {
        /*
         * Brak parametru -o:
         * wynik pozostaje w terminalu.
         */
        if (!redirectRequested) {
            return;
        }

        outputFile.open(
            filePath,
            std::ios::out | std::ios::trunc
        );

        if (!outputFile.is_open()) {
            return;
        }

        /*
         * Przekierowanie std::cout do pliku.
         */
        originalBuffer = std::cout.rdbuf(
            outputFile.rdbuf()
        );

        redirectSuccessful = true;
    }

    ~CoutRedirect() {
        /*
         * Najpierw przywracamy terminal,
         * dopiero później zamykamy plik.
         */
        if (redirectSuccessful) {
            std::cout.flush();
            std::cout.rdbuf(originalBuffer);
        }

        if (outputFile.is_open()) {
            outputFile.close();
        }
    }

    bool wasRequested() const {
        return redirectRequested;
    }

    bool isSuccessful() const {
        return redirectSuccessful;
    }
};


int runSingleFileMode() {
    GraphData data;

    if (Parameters::inputFile.empty()) {
        std::cout << "Error: input file path is empty.\n";
        return 1;
    }

    if (!GraphLoader::load(Parameters::inputFile, data)) {
        std::cout << "Error: cannot load input graph.\n";
        return 1;
    }

    if (!validateSingleFileParameters(data)) {
        std::cout
            << "Single file mode was not started "
            << "because parameters are invalid.\n";

        GraphLoader::free(data);
        return 1;
    }

    if (!Parameters::resultsFile.empty()) {
        if (!Parameters::outputFile.empty()) {
            std::cout
                << "Error: use either outputFile (-o) "
                << "or resultsFile (-r), not both.\n";

            GraphLoader::free(data);
            return 1;
        }

        const bool success =
            runLoadedGraphBenchmark(data);

        GraphLoader::free(data);

        return success ? 0 : 1;
    }

    CoutRedirect outputRedirect(
        Parameters::outputFile
    );

    if (
        outputRedirect.wasRequested() &&
        !outputRedirect.isSuccessful()
    ) {
        std::cout
            << "Error: cannot open output file: "
            << Parameters::outputFile
            << "\n";

        GraphLoader::free(data);
        return 1;
    }

    std::cout << "Single file mode selected.\n";
    std::cout << "Input file: "
              << Parameters::inputFile
              << "\n";
    std::cout << "Problem: "
              << getProblemName(Parameters::problem)
              << "\n";
    std::cout << "Algorithm: "
              << getAlgorithmName(Parameters::algorithm)
              << "\n";
    std::cout << "Structure: "
              << getStructureName(Parameters::structure)
              << "\n";

    std::cout << "\nLoaded graph:\n";
    std::cout << "Vertices: "
              << data.vertexCount
              << "\n";
    std::cout << "Edges: "
              << data.edgeCount
              << "\n";

    bool success = true;

    if (
        Parameters::structure ==
        Parameters::Structures::allStructures
    ) {
        if (!runSingleFileForStructure(
                data,
                Parameters::Structures::incidenceMatrix
            )) {
            success = false;
            }

        if (!runSingleFileForStructure(
                data,
                Parameters::Structures::adjacencyList
            )) {
            success = false;
            }
    } else {
        success = runSingleFileForStructure(
            data,
            Parameters::structure
        );
    }

    GraphLoader::free(data);

    return success ? 0 : 1;
}

bool runBenchmarkCase(
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

        const unsigned int seed = createBenchmarkSeed(
            problem,
            vertexCount,
            density,
            iteration
        );

        std::srand(seed);

        GraphData data = GraphGenerator::generate(
            vertexCount,
            density,
            directed
        );

        lastEdgeCount = data.edgeCount;

        long long timeUs = 0;

        if (structure == Parameters::Structures::adjacencyList) {
            AdjacencyListGraph graph(data.vertexCount);

            if (!GraphBuilder::buildAdjacencyListGraph(
                    data,
                    graph,
                    undirected
                )) {
                std::cout
                    << "Error: cannot build adjacency list graph "
                    << "during benchmark.\n";

                GraphLoader::free(data);
                return false;
                }

            if (
                graph.getVertexCount() != data.vertexCount ||
                graph.getEdgeCount() != data.edgeCount
            ) {
                std::cout
                    << "Error: adjacency list verification failed "
                    << "during benchmark.\n";

                GraphLoader::free(data);
                return false;
            }

            timeUs = measureAlgorithm(
                graph,
                problem,
                algorithm,
                startVertex,
                endVertex
            );
        } else if (structure == Parameters::Structures::incidenceMatrix) {
            IncidenceMatrixGraph graph(
            data.vertexCount,
            data.edgeCount
            );

            if (!GraphBuilder::buildIncidenceMatrixGraph(
                    data,
                    graph,
                    undirected
                )) {
                std::cout
                    << "Error: cannot build incidence matrix graph "
                    << "during benchmark.\n";

                GraphLoader::free(data);
                return false;
                }

            if (
                graph.getVertexCount() != data.vertexCount ||
                graph.getEdgeCount() != data.edgeCount
            ) {
                std::cout
                    << "Error: incidence matrix verification failed "
                    << "during benchmark.\n";

                GraphLoader::free(data);
                return false;
            }

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

    if (!CsvLogger::appendResult(outputFile, result)) {
        std::cout
            << "Error: benchmark was calculated, "
            << "but the result was not saved.\n";

        return false;
    }

    std::cout << "Saved benchmark: "
              << result.problem << "; "
              << result.algorithm << "; "
              << result.structure << "; V="
              << result.vertexCount << "; density="
              << result.density << "%; avg="
              << result.avgTimeUs << " us\n";

    return true;
}

bool runAlgorithmsForProblem(
    const Parameters::Problems problem,
    const Parameters::Structures structure,
    const int vertexCount,
    const int density,
    const int iterations,
    const std::string& outputFile
) {
    bool success = true;

    if (problem == Parameters::Problems::mst) {
        if (!runBenchmarkCase(
                problem,
                Parameters::Algorithms::prim,
                structure,
                vertexCount,
                density,
                iterations,
                outputFile
            )) {
            success = false;
        }

        if (!runBenchmarkCase(
                problem,
                Parameters::Algorithms::kruskal,
                structure,
                vertexCount,
                density,
                iterations,
                outputFile
            )) {
            success = false;
        }
    } else if (problem == Parameters::Problems::sp) {
        if (!runBenchmarkCase(
                problem,
                Parameters::Algorithms::dijkstra,
                structure,
                vertexCount,
                density,
                iterations,
                outputFile
            )) {
            success = false;
        }

        if (!runBenchmarkCase(
                problem,
                Parameters::Algorithms::bellmanFord,
                structure,
                vertexCount,
                density,
                iterations,
                outputFile
            )) {
            success = false;
        }
    } else if (problem == Parameters::Problems::mf) {
        if (!runBenchmarkCase(
                problem,
                Parameters::Algorithms::fordFulkerson,
                structure,
                vertexCount,
                density,
                iterations,
                outputFile
            )) {
            success = false;
        }
    } else {
        std::cout << "Error: unsupported benchmark problem.\n";
        return false;
    }

    return success;
}

int runBenchmarkMode() {
    if (!validateBenchmarkParameters()) {
        std::cout << "Benchmark was not started because parameters are invalid.\n";
        return 1;
    }

    std::cout << "Benchmark mode selected.\n";
    std::cout << "Problem: " << getProblemName(Parameters::problem) << "\n";
    std::cout << "Algorithm: " << getAlgorithmName(Parameters::algorithm) << "\n";
    std::cout << "Structure: " << getStructureName(Parameters::structure) << "\n";
    std::cout << "VertexCount: " << Parameters::vertexCount << "\n";
    std::cout << "Density: " << Parameters::density << "%\n";
    std::cout << "Iterations: " << Parameters::iterations << "\n";
    std::cout << "Results file: " << Parameters::resultsFile << "\n";

    if (
        Parameters::algorithm ==
            Parameters::Algorithms::allAlgorithms &&
        Parameters::structure ==
            Parameters::Structures::allStructures
    ) {
        bool success = true;

        if (!runAlgorithmsForProblem(
                Parameters::problem,
                Parameters::Structures::incidenceMatrix,
                Parameters::vertexCount,
                Parameters::density,
                Parameters::iterations,
                Parameters::resultsFile
            )) {
            success = false;
        }

        if (!runAlgorithmsForProblem(
                Parameters::problem,
                Parameters::Structures::adjacencyList,
                Parameters::vertexCount,
                Parameters::density,
                Parameters::iterations,
                Parameters::resultsFile
            )) {
            success = false;
        }

        return success ? 0 : 1;
    }

    // Wszystkie algorytmy dla jednej reprezentacji
    if (
        Parameters::algorithm ==
        Parameters::Algorithms::allAlgorithms
    ) {
        const bool success = runAlgorithmsForProblem(
            Parameters::problem,
            Parameters::structure,
            Parameters::vertexCount,
            Parameters::density,
            Parameters::iterations,
            Parameters::resultsFile
        );

        return success ? 0 : 1;
    }

    // Jeden algorytm dla wszystkich reprezentacji
    if (
        Parameters::structure ==
        Parameters::Structures::allStructures
    ) {
        bool success = true;

        if (!runBenchmarkCase(
                Parameters::problem,
                Parameters::algorithm,
                Parameters::Structures::incidenceMatrix,
                Parameters::vertexCount,
                Parameters::density,
                Parameters::iterations,
                Parameters::resultsFile
            )) {
            success = false;
        }

        if (!runBenchmarkCase(
                Parameters::problem,
                Parameters::algorithm,
                Parameters::Structures::adjacencyList,
                Parameters::vertexCount,
                Parameters::density,
                Parameters::iterations,
                Parameters::resultsFile
            )) {
            success = false;
        }

        return success ? 0 : 1;
    }

    //Jeden algorytm dla jednej reprezentacji
    const bool success = runBenchmarkCase(
        Parameters::problem,
        Parameters::algorithm,
        Parameters::structure,
        Parameters::vertexCount,
        Parameters::density,
        Parameters::iterations,
        Parameters::resultsFile
    );

    return success ? 0 : 1;
}

void printAdditionalHelp() {
    std::cout
        << "\nDataset benchmark:\n"
        << "  Use --singleFile together with -r and -n.\n"
        << "  Example:\n"
        << "  ./aizo-project2 --singleFile -p 1 -a 0 -s 0 "
        << "-i data/dataset.txt -c 0 -e 999 "
        << "-n 100 -r results/dataset.csv\n";
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
        printAdditionalHelp();
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
