import argparse
import subprocess
from pathlib import Path


DEFAULT_EXECUTABLE = "./cmake-build-debug/aizo-project2"

DIRECTED_DATASET = "data/datasets/email_Eu_core_directed_weighted.txt"
UNDIRECTED_DATASET = "data/datasets/email_Eu_core_undirected_weighted.txt"

RESULTS_DIR = Path("results/email_dataset")


PROBLEMS = {
    "mst": 0,
    "sp": 1,
    "mf": 2,
}

ALGORITHMS = {
    "prim": 1,
    "kruskal": 2,
    "dijkstra": 3,
    "bellman_ford": 4,
    "ford_fulkerson": 5,
}

STRUCTURES = {
    "matrix": 1,
    "list": 2,
}


def check_file_exists(path: str, description: str) -> None:
    if not Path(path).exists():
        raise FileNotFoundError(f"Missing {description}: {path}")


def run_command(command: list[str], output_path: Path) -> bool:
    print("Running:", " ".join(command))
    print("Output:", output_path)

    output_path.parent.mkdir(parents=True, exist_ok=True)

    with output_path.open("w", encoding="utf-8") as output_file:
        process = subprocess.run(
            command,
            stdout=output_file,
            stderr=subprocess.STDOUT,
            text=True,
        )

    if process.returncode != 0:
        print(f"FAILED: {output_path}")
        return False

    print(f"OK: {output_path}")
    return True


def build_single_file_command(
        executable: str,
        input_file: str,
        problem: int,
        algorithm: int,
        structure: int,
        source: int | None = None,
        target: int | None = None,
) -> list[str]:
    command = [
        executable,
        "--singleFile",
        "--inputFile",
        input_file,
        "-p",
        str(problem),
        "-a",
        str(algorithm),
        "-s",
        str(structure),
    ]

    if source is not None and target is not None:
        command.extend(["-c", str(source), "-e", str(target)])

    return command


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Run email-Eu-core dataset on multiple graph algorithms."
    )

    parser.add_argument(
        "--executable",
        default=DEFAULT_EXECUTABLE,
        help="Path to compiled C++ executable.",
    )

    parser.add_argument(
        "--source",
        type=int,
        default=0,
        help="Source/start vertex for SP and MF.",
    )

    parser.add_argument(
        "--target",
        type=int,
        default=999,
        help="Target/end/sink vertex for SP and MF.",
    )

    parser.add_argument(
        "--structures",
        choices=["list", "matrix", "both"],
        default="list",
        help="Graph representation to test. Default: list.",
    )

    parser.add_argument(
        "--skip-mf",
        action="store_true",
        help="Skip Ford-Fulkerson, useful if maximum flow is too slow.",
    )

    args = parser.parse_args()

    check_file_exists(args.executable, "compiled executable")
    check_file_exists(DIRECTED_DATASET, "directed weighted dataset")
    check_file_exists(UNDIRECTED_DATASET, "undirected weighted dataset")

    RESULTS_DIR.mkdir(parents=True, exist_ok=True)

    if args.structures == "both":
        selected_structures = ["list", "matrix"]
    else:
        selected_structures = [args.structures]

    cases = []

    for structure_name in selected_structures:
        structure_value = STRUCTURES[structure_name]

        cases.append(
            {
                "name": f"mst_prim_{structure_name}",
                "input": UNDIRECTED_DATASET,
                "problem": PROBLEMS["mst"],
                "algorithm": ALGORITHMS["prim"],
                "structure": structure_value,
                "source": None,
                "target": None,
            }
        )

        cases.append(
            {
                "name": f"mst_kruskal_{structure_name}",
                "input": UNDIRECTED_DATASET,
                "problem": PROBLEMS["mst"],
                "algorithm": ALGORITHMS["kruskal"],
                "structure": structure_value,
                "source": None,
                "target": None,
            }
        )

        cases.append(
            {
                "name": f"sp_dijkstra_{structure_name}",
                "input": DIRECTED_DATASET,
                "problem": PROBLEMS["sp"],
                "algorithm": ALGORITHMS["dijkstra"],
                "structure": structure_value,
                "source": args.source,
                "target": args.target,
            }
        )

        cases.append(
            {
                "name": f"sp_bellman_ford_{structure_name}",
                "input": DIRECTED_DATASET,
                "problem": PROBLEMS["sp"],
                "algorithm": ALGORITHMS["bellman_ford"],
                "structure": structure_value,
                "source": args.source,
                "target": args.target,
            }
        )

        if not args.skip_mf:
            cases.append(
                {
                    "name": f"mf_ford_fulkerson_{structure_name}",
                    "input": DIRECTED_DATASET,
                    "problem": PROBLEMS["mf"],
                    "algorithm": ALGORITHMS["ford_fulkerson"],
                    "structure": structure_value,
                    "source": args.source,
                    "target": args.target,
                }
            )

    success_count = 0

    for case in cases:
        output_path = RESULTS_DIR / f"{case['name']}.txt"

        command = build_single_file_command(
            executable=args.executable,
            input_file=case["input"],
            problem=case["problem"],
            algorithm=case["algorithm"],
            structure=case["structure"],
            source=case["source"],
            target=case["target"],
        )

        if run_command(command, output_path):
            success_count += 1

    print()
    print("Finished dataset run.")
    print(f"Successful runs: {success_count}/{len(cases)}")
    print(f"Results directory: {RESULTS_DIR}")


if __name__ == "__main__":
    main()