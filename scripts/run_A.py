import subprocess
from pathlib import Path


EXECUTABLE = "./cmake-build-debug/aizo-project2"
OUTPUT_FILE = "results/benchmark_A.csv"

VERTEX_COUNTS = [50, 100, 150, 200, 250]
DENSITY = 50
ITERATIONS = 100

PROBLEMS = [
    0,  # MST
    1,  # SP
    2,  # MF
]


def run_command(command: list[str]) -> None:

    print("Running:", " ".join(command))

    subprocess.run(
        command,
        check=True,
    )


def main() -> None:
    Path(OUTPUT_FILE).parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    for vertex_count in VERTEX_COUNTS:
        for problem in PROBLEMS:
            command = [
                EXECUTABLE,
                "--benchmark",

                "-p",
                str(problem),

                # Wszystkie algorytmy pasujące do problemu
                "-a",
                "0",

                # Obie reprezentacje grafu
                "-s",
                "0",

                "-l",
                str(vertex_count),

                "-d",
                str(DENSITY),

                "-n",
                str(ITERATIONS),

                "-r",
                OUTPUT_FILE,
            ]

            run_command(command)


if __name__ == "__main__":
    main()