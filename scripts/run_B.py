import subprocess
from pathlib import Path


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

    for density in DENSITIES:
        for problem in PROBLEMS:
            command = [
                EXECUTABLE,
                "--benchmark",

                "-p",
                str(problem),

                # Wszystkie algorytmy dla danego problemu.
                "-a",
                "0",

                # Obie reprezentacje.
                "-s",
                "0",

                "-l",
                str(VERTEX_COUNT),

                "-d",
                str(density),

                "-n",
                str(ITERATIONS),

                "-r",
                OUTPUT_FILE,
            ]

            run_command(command)


if __name__ == "__main__":
    main()