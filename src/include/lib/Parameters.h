#ifndef PARAMETERS_H
#define PARAMETERS_H

#define LIB_VERSION "v.1.3"

#include <string>

namespace Parameters
{
    enum class RunModes {
        undefined = -1,
        singleFile = 0,
        benchmark,
        help
      };

    enum class Problems {
        undefined = -1,
        mst = 0,
        sp,
        mf,

        count
      };

    enum class Algorithms {
        undefined = -1,
        allAlgorithms = 0,
        prim,
        kruskal,
        dijkstra,
        bellmanFord,
        fordFulkerson,

        count
      };

    enum class Structures {
        undefined = -1,
        allStructures = 0,
        incidenceMatrix,
        adjacencyList,

        count
      };

    //
    // Parameters to use.
    //

    extern RunModes runMode;
    extern Problems problem;
    extern Algorithms algorithm;
    extern Structures structure;

    extern std::string inputFile;
    extern std::string outputFile;
    extern std::string resultsFile;

    extern int vertexStart;
    extern int vertexEnd;

    extern int vertexCount;
    extern int density;
    extern int iterations;

    //
    // Methods to use.
    //

    int readParameters(int argc, char **argv);
    void printParameters();

    void enableDebug();
    void disableDebug();

    void help();
    const char *getVersion();
};

#endif