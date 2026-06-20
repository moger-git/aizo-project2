import argparse
import subprocess
from pathlib import Path


DEFAULT_EXECUTABLE = "./cmake-build-debug/aizo-project2"

DIRECTED_DATASET = (
    "data/datasets/"
    "email_Eu_core_directed_weighted.txt"
)

UNDIRECTED_DATASET = (
    "data/datasets/"
    "email_Eu_core_mst_connected_weighted.txt"
)

DEFAULT_CSV_FILE = (
    "results/email_dataset/"
    "email_dataset_results.csv"
)


STRUCTURES = {
    "both": 0,
    "matrix": 1,
    "list": 2,
}


def check_file(path: str, description: str) -> None:
    if not Path(path).exists():
        raise FileNotFoundError(
            f"Missing {description}: {path}"
        )


def run_command(command: list[str]) -> None:
    print()
    print("Running:")
    print(" ".join(command))

    subprocess.run(
        command,
        check=True,
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Run the email-Eu-core dataset. "
            "Times are measured inside C++ and include "
            "only the selected graph algorithm."
        )
    )

    parser.add_argument(
        "--executable",
        default=DEFAULT_EXECUTABLE,
        help="Path to the compiled C++ executable.",
    )

    parser.add_argument(
        "--source",
        type=int,
        default=0,
        help="Source vertex for SP and MF.",
    )

    parser.add_argument(
        "--target",
        type=int,
        default=999,
        help="Target vertex for SP and MF.",
    )

    parser.add_argument(
        "--structures",
        choices=["both", "matrix", "list"],
        default="both",
        help="Representations to benchmark.",
    )

    parser.add_argument(
        "--iterations",
        type=int,
        default=100,
        help="Number of repetitions inside C++.",
    )

    parser.add_argument(
        "--csv",
        default=DEFAULT_CSV_FILE,
        help="Output CSV file.",
    )

    parser.add_argument(
        "--skip-mf",
        action="store_true",
        help="Skip Ford-Fulkerson.",
    )

    parser.add_argument(
        "--clear",
        action="store_true",
        help="Delete the old CSV before running.",
    )

    args = parser.parse_args()

    check_file(
        args.executable,
        "compiled executable",
    )

    check_file(
        DIRECTED_DATASET,
        "directed dataset",
    )

    check_file(
        UNDIRECTED_DATASET,
        "undirected dataset",
    )

    if args.iterations <= 0:
        raise ValueError(
            "Iterations must be greater than 0."
        )

    csv_path = Path(args.csv)
    csv_path.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    if args.clear and csv_path.exists():
        csv_path.unlink()

    structure = STRUCTURES[args.structures]

    # MST: Prim i Kruskal, obie wybrane reprezentacje.
    mst_command = [
        args.executable,
        "--singleFile",
        "-p",
        "0",
        "-a",
        "0",
        "-s",
        str(structure),
        "-i",
        UNDIRECTED_DATASET,
        "-n",
        str(args.iterations),
        "-r",
        str(csv_path),
    ]

    run_command(mst_command)

    # SP: Dijkstra i Bellman-Ford.
    sp_command = [
        args.executable,
        "--singleFile",
        "-p",
        "1",
        "-a",
        "0",
        "-s",
        str(structure),
        "-i",
        DIRECTED_DATASET,
        "-c",
        str(args.source),
        "-e",
        str(args.target),
        "-n",
        str(args.iterations),
        "-r",
        str(csv_path),
    ]

    run_command(sp_command)

    # MF: Ford-Fulkerson.
    if not args.skip_mf:
        mf_command = [
            args.executable,
            "--singleFile",
            "-p",
            "2",
            "-a",
            "5",
            "-s",
            str(structure),
            "-i",
            DIRECTED_DATASET,
            "-c",
            str(args.source),
            "-e",
            str(args.target),
            "-n",
            str(args.iterations),
            "-r",
            str(csv_path),
        ]

        run_command(mf_command)

    print()
    print("Dataset benchmark finished.")
    print(f"CSV saved to: {csv_path}")


if __name__ == "__main__":
    main()