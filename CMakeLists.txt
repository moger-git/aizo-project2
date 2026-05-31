cmake_minimum_required(VERSION 3.16)

project(aizo-project2 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(aizo-project2
        src/main.cpp
        src/main.h
        src/include/io/CsvLogger.h
        src/include/algorithms/Prim.h
        src/include/algorithms/Kruskal.h
        src/include/algorithms/Dijkstra.h
        src/include/algorithms/BellmanFord.h
        src/include/algorithms/FordFulkerson.h
        src/include/graph/Edge.h
        src/include/graph/IncidenceMatrixGraph.h
        src/include/graph/AdjacencyListGraph.h
        src/include/utils/Timer.h
        src/include/io/GraphLoader.h
        src/include/graph/GraphData.h
        src/include/graph/GraphBuilder.h
        src/include/io/DateTime.h
        src/include/graph/GraphGenerator.h
)

target_include_directories(aizo-project2 PUBLIC src src/include)

target_link_libraries(aizo-project2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/libparameters.so)

target_compile_options(aizo-project2 PRIVATE
        -Wall
        -Wextra
        -Werror
)
