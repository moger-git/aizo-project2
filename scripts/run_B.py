import subprocess

EXECUTABLE = "./cmake-build-debug/aizo-project2"
OUTPUT_FILE = "results/benchmark_B.csv"

VERTEX_COUNT = 150
DENSITIES = [25, 50, 99]
ITERATIONS = 100

PROBLEMS = [
    0,  # MST
    1,  # SP
    2,  # MF
]

ALGORITHMS_BY_PROBLEM = {
    0: [1, 2],  # Prim, Kruskal
    1: [3, 4],  # Dijkstra, Bellman-Ford
    2: [5],     # Ford-Fulkerson
}

STRUCTURES = [
    1,  # incidenceMatrix
    2,  # adjacencyList
]


def run_command(command):
    print("Running:", " ".join(command))
    subprocess.run(command, check=True)


def main():
    for density in DENSITIES:
        for problem in PROBLEMS:
            for algorithm in ALGORITHMS_BY_PROBLEM[problem]:
                for structure in STRUCTURES:
                    command = [
                        EXECUTABLE,
                        "--benchmark",
                        "-p", str(problem),
                        "-a", str(algorithm),
                        "-s", str(structure),
                        "-l", str(VERTEX_COUNT),
                        "-d", str(density),
                        "-n", str(ITERATIONS),
                        "-r", OUTPUT_FILE,
                    ]

                    run_command(command)


if __name__ == "__main__":
    main()