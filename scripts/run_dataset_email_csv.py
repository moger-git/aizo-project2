import argparse
import csv
import subprocess
import time
from datetime import datetime
from pathlib import Path


DEFAULT_EXECUTABLE = "./cmake-build-debug/aizo-project2"

DIRECTED_DATASET = "data/datasets/email_Eu_core_directed_weighted.txt"
UNDIRECTED_DATASET = "data/datasets/email_Eu_core_undirected_weighted.txt"

RESULTS_DIR = Path("results/email_dataset")
DEFAULT_CSV_FILE = RESULTS_DIR / "email_dataset_results.csv"


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


def read_graph_size(input_file: str) -> tuple[int, int]:
    with Path(input_file).open("r", encoding="utf-8") as file:
        first_line = file.readline().split()

    if len(first_line) < 2:
        raise ValueError(f"Invalid graph file header: {input_file}")

    vertex_count = int(first_line[0])
    edge_count = int(first_line[1])

    return vertex_count, edge_count


def calculate_density(vertex_count: int, edge_count: int, directed: bool) -> float:
    if vertex_count <= 1:
        return 0.0

    if directed:
        max_edges = vertex_count * (vertex_count - 1)
    else:
        max_edges = vertex_count * (vertex_count - 1) // 2

    if max_edges == 0:
        return 0.0

    return (edge_count / max_edges) * 100.0


def build_command(
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


def measure_command(command: list[str], output_file: Path) -> tuple[bool, int]:
    start_time = time.perf_counter_ns()

    process = subprocess.run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )

    end_time = time.perf_counter_ns()

    time_us = (end_time - start_time) // 1000

    output_file.parent.mkdir(parents=True, exist_ok=True)
    output_file.write_text(process.stdout, encoding="utf-8")

    return process.returncode == 0, time_us


def append_csv_row(csv_path: Path, row: dict) -> None:
    csv_path.parent.mkdir(parents=True, exist_ok=True)

    file_exists = csv_path.exists()

    fieldnames = [
        "date",
        "problem",
        "algorithm",
        "structure",
        "vertexCount",
        "edgeCount",
        "density",
        "iterations",
        "minTimeUs",
        "maxTimeUs",
        "avgTimeUs",
    ]

    with csv_path.open("a", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames, delimiter=";")

        if not file_exists:
            writer.writeheader()

        writer.writerow(row)


def run_case(
        case: dict,
        executable: str,
        iterations: int,
        csv_path: Path,
) -> None:
    vertex_count, edge_count = read_graph_size(case["input_file"])

    density = calculate_density(
        vertex_count=vertex_count,
        edge_count=edge_count,
        directed=case["directed"],
    )

    command = build_command(
        executable=executable,
        input_file=case["input_file"],
        problem=case["problem_id"],
        algorithm=case["algorithm_id"],
        structure=case["structure_id"],
        source=case["source"],
        target=case["target"],
    )

    times = []
    successful_runs = 0

    print()
    print(f"Case: {case['problem_name']} | {case['algorithm_name']} | {case['structure_name']}")
    print("Command:", " ".join(command))

    for iteration in range(iterations):
        output_file = RESULTS_DIR / "outputs" / f"{case['file_name']}_run_{iteration + 1}.txt"

        ok, time_us = measure_command(command, output_file)

        if ok:
            successful_runs += 1
            times.append(time_us)
            print(f"  run {iteration + 1}/{iterations}: {time_us} us")
        else:
            print(f"  run {iteration + 1}/{iterations}: FAILED, output={output_file}")

    if not times:
        min_time = 0
        max_time = 0
        avg_time = 0.0
    else:
        min_time = min(times)
        max_time = max(times)
        avg_time = sum(times) / len(times)

    row = {
        "date": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "problem": case["problem_name"],
        "algorithm": case["algorithm_name"],
        "structure": case["structure_name"],
        "vertexCount": vertex_count,
        "edgeCount": edge_count,
        "density": f"{density:.6f}",
        "iterations": successful_runs,
        "minTimeUs": min_time,
        "maxTimeUs": max_time,
        "avgTimeUs": f"{avg_time:.2f}",
    }

    append_csv_row(csv_path, row)

    print(
        f"Saved CSV row: {case['problem_name']}; "
        f"{case['algorithm_name']}; "
        f"{case['structure_name']}; "
        f"min={min_time}; max={max_time}; avg={avg_time:.2f}"
    )


def build_cases(
        structures: list[str],
        source: int,
        target: int,
        skip_mf: bool,
) -> list[dict]:
    cases = []

    for structure_name in structures:
        structure_id = STRUCTURES[structure_name]

        if structure_name == "list":
            structure_label = "AdjacencyList"
        else:
            structure_label = "IncidenceMatrix"

        cases.append(
            {
                "file_name": f"mst_prim_{structure_name}",
                "problem_name": "MST",
                "algorithm_name": "Prim",
                "structure_name": structure_label,
                "input_file": UNDIRECTED_DATASET,
                "problem_id": PROBLEMS["mst"],
                "algorithm_id": ALGORITHMS["prim"],
                "structure_id": structure_id,
                "source": None,
                "target": None,
                "directed": False,
            }
        )

        cases.append(
            {
                "file_name": f"mst_kruskal_{structure_name}",
                "problem_name": "MST",
                "algorithm_name": "Kruskal",
                "structure_name": structure_label,
                "input_file": UNDIRECTED_DATASET,
                "problem_id": PROBLEMS["mst"],
                "algorithm_id": ALGORITHMS["kruskal"],
                "structure_id": structure_id,
                "source": None,
                "target": None,
                "directed": False,
            }
        )

        cases.append(
            {
                "file_name": f"sp_dijkstra_{structure_name}",
                "problem_name": "SP",
                "algorithm_name": "Dijkstra",
                "structure_name": structure_label,
                "input_file": DIRECTED_DATASET,
                "problem_id": PROBLEMS["sp"],
                "algorithm_id": ALGORITHMS["dijkstra"],
                "structure_id": structure_id,
                "source": source,
                "target": target,
                "directed": True,
            }
        )

        cases.append(
            {
                "file_name": f"sp_bellman_ford_{structure_name}",
                "problem_name": "SP",
                "algorithm_name": "Bellman-Ford",
                "structure_name": structure_label,
                "input_file": DIRECTED_DATASET,
                "problem_id": PROBLEMS["sp"],
                "algorithm_id": ALGORITHMS["bellman_ford"],
                "structure_id": structure_id,
                "source": source,
                "target": target,
                "directed": True,
            }
        )

        if not skip_mf:
            cases.append(
                {
                    "file_name": f"mf_ford_fulkerson_{structure_name}",
                    "problem_name": "MF",
                    "algorithm_name": "Ford-Fulkerson",
                    "structure_name": structure_label,
                    "input_file": DIRECTED_DATASET,
                    "problem_id": PROBLEMS["mf"],
                    "algorithm_id": ALGORITHMS["ford_fulkerson"],
                    "structure_id": structure_id,
                    "source": source,
                    "target": target,
                    "directed": True,
                }
            )

    return cases


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Run email-Eu-core dataset and save results in benchmark CSV format."
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
        help="Graph representation to test.",
    )

    parser.add_argument(
        "--iterations",
        type=int,
        default=3,
        help="How many times each case should be repeated.",
    )

    parser.add_argument(
        "--skip-mf",
        action="store_true",
        help="Skip Ford-Fulkerson if it is too slow.",
    )

    parser.add_argument(
        "--csv",
        default=str(DEFAULT_CSV_FILE),
        help="Output CSV file.",
    )

    parser.add_argument(
        "--clear",
        action="store_true",
        help="Remove existing CSV file before running.",
    )

    args = parser.parse_args()

    check_file_exists(args.executable, "compiled executable")
    check_file_exists(DIRECTED_DATASET, "directed weighted dataset")
    check_file_exists(UNDIRECTED_DATASET, "undirected weighted dataset")

    RESULTS_DIR.mkdir(parents=True, exist_ok=True)

    csv_path = Path(args.csv)

    if args.clear and csv_path.exists():
        csv_path.unlink()

    if args.structures == "both":
        selected_structures = ["list", "matrix"]
    else:
        selected_structures = [args.structures]

    cases = build_cases(
        structures=selected_structures,
        source=args.source,
        target=args.target,
        skip_mf=args.skip_mf,
    )

    for case in cases:
        run_case(
            case=case,
            executable=args.executable,
            iterations=args.iterations,
            csv_path=csv_path,
        )

    print()
    print("Finished.")
    print(f"CSV saved to: {csv_path}")


if __name__ == "__main__":
    main()