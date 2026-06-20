from __future__ import annotations

import argparse
import csv
import re
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Callable

import networkx as nx
from networkx.algorithms.flow import preflow_push


DEFAULT_EXECUTABLE = Path("./cmake-build-debug/aizo-project2")

MST_DATASET = Path(
    "data/datasets/email_Eu_core_mst_connected_weighted.txt"
)

DIRECTED_DATASET = Path(
    "data/datasets/email_Eu_core_directed_weighted.txt"
)

DEFAULT_PROJECT_CSV = Path(
    "results/email_dataset/project_dataset_times.csv"
)

DEFAULT_COMPARISON_CSV = Path(
    "results/email_dataset/comparison_networkx.csv"
)


@dataclass(frozen=True)
class Case:
    problem: str
    algorithm: str
    problem_id: int
    algorithm_id: int
    dataset: Path


CASES = (
    Case("MST", "Prim", 0, 1, MST_DATASET),
    Case("MST", "Kruskal", 0, 2, MST_DATASET),
    Case("SP", "Dijkstra", 1, 3, DIRECTED_DATASET),
    Case("SP", "Bellman-Ford", 1, 4, DIRECTED_DATASET),
    Case("MF", "Ford-Fulkerson", 2, 5, DIRECTED_DATASET),
)

STRUCTURES = (
    ("IncidenceMatrix", 1),
    ("AdjacencyList", 2),
)


class ComparisonError(RuntimeError):
    """Błąd wykonywania porównania."""


def check_file(path: Path, description: str) -> None:
    if not path.is_file():
        raise ComparisonError(f"Missing {description}: {path}")


def read_graph_file(
        path: Path,
) -> tuple[int, int, list[tuple[int, int, int]]]:
    """Wczytuje graf w formacie używanym przez projekt."""

    with path.open("r", encoding="utf-8") as file:
        header = file.readline().split()

        if len(header) != 2:
            raise ComparisonError(f"Invalid header in {path}")

        vertex_count = int(header[0])
        declared_edge_count = int(header[1])
        edges: list[tuple[int, int, int]] = []

        for line_number, line in enumerate(file, start=2):
            parts = line.split()

            if not parts:
                continue

            if len(parts) != 3:
                raise ComparisonError(
                    f"Invalid edge at {path}:{line_number}"
                )

            source, target, value = map(int, parts)

            if not (
                    0 <= source < vertex_count
                    and 0 <= target < vertex_count
            ):
                raise ComparisonError(
                    f"Invalid vertex at {path}:{line_number}"
                )

            edges.append((source, target, value))

    if len(edges) != declared_edge_count:
        raise ComparisonError(
            f"Edge count mismatch in {path}: "
            f"declared={declared_edge_count}, "
            f"read={len(edges)}"
        )

    return vertex_count, declared_edge_count, edges


def build_mst_graph(
        vertex_count: int,
        edges: list[tuple[int, int, int]],
) -> nx.Graph:
    """
    Buduje graf nieskierowany.

    Przy powtórzonej krawędzi zachowuje najmniejszą wagę.
    """

    graph = nx.Graph()
    graph.add_nodes_from(range(vertex_count))

    for source, target, weight in edges:
        if graph.has_edge(source, target):
            previous_weight = graph[source][target]["weight"]

            if weight < previous_weight:
                graph[source][target]["weight"] = weight
        else:
            graph.add_edge(
                source,
                target,
                weight=weight,
            )

    if not nx.is_connected(graph):
        raise ComparisonError(
            "MST dataset is not connected. "
            "Use the connected dataset prepared in part 9."
        )

    return graph


def build_shortest_path_graph(
        vertex_count: int,
        edges: list[tuple[int, int, int]],
) -> nx.DiGraph:
    """
    Buduje graf skierowany dla SP.

    Przy równoległych krawędziach zachowuje najmniejszą wagę.
    """

    graph = nx.DiGraph()
    graph.add_nodes_from(range(vertex_count))

    for source, target, weight in edges:
        if graph.has_edge(source, target):
            previous_weight = graph[source][target]["weight"]

            if weight < previous_weight:
                graph[source][target]["weight"] = weight
        else:
            graph.add_edge(
                source,
                target,
                weight=weight,
            )

    return graph


def build_flow_graph(
        vertex_count: int,
        edges: list[tuple[int, int, int]],
) -> nx.DiGraph:
    """
    Buduje graf przepływowy.

    Przy równoległych krawędziach sumuje przepustowości,
    tak samo jak implementacja projektu.
    """

    graph = nx.DiGraph()
    graph.add_nodes_from(range(vertex_count))

    for source, target, capacity in edges:
        if graph.has_edge(source, target):
            graph[source][target]["capacity"] += capacity
        else:
            graph.add_edge(
                source,
                target,
                capacity=capacity,
            )

    return graph


def command_for_case(
        executable: Path,
        case: Case,
        structure_id: int,
        source: int,
        target: int,
) -> list[str]:
    command = [
        str(executable),
        "--singleFile",
        "-p",
        str(case.problem_id),
        "-a",
        str(case.algorithm_id),
        "-s",
        str(structure_id),
        "-i",
        str(case.dataset),
    ]

    if case.problem in {"SP", "MF"}:
        command.extend(
            [
                "-c",
                str(source),
                "-e",
                str(target),
            ]
        )

    return command


def run_project_benchmarks(
        executable: Path,
        project_csv: Path,
        iterations: int,
        source: int,
        target: int,
        skip_mf: bool,
) -> None:
    """
    Uruchamia pomiary projektu.

    Czas jest mierzony wewnątrz C++, dlatego nie obejmuje:
    - uruchomienia procesu,
    - wczytania datasetu,
    - budowania reprezentacji,
    - wypisywania wyniku.
    """

    project_csv.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    if project_csv.exists():
        project_csv.unlink()

    for case in CASES:
        if skip_mf and case.problem == "MF":
            continue

        for structure_name, structure_id in STRUCTURES:
            command = command_for_case(
                executable,
                case,
                structure_id,
                source,
                target,
            )

            command.extend(
                [
                    "-n",
                    str(iterations),
                    "-r",
                    str(project_csv),
                ]
            )

            print(
                f"Project benchmark: "
                f"{case.problem} | {case.algorithm} | "
                f"{structure_name}"
            )

            subprocess.run(
                command,
                check=True,
            )


def parse_project_result(
        problem: str,
        output: str,
) -> int:
    patterns = {
        "MST": r"MST cost:\s*(-?\d+)",
        "SP": r"Cost:\s*(-?\d+)",
        "MF": (
            r"Maximum flow from\s+\d+\s+to\s+\d+"
            r":\s*(-?\d+)"
        ),
    }

    match = re.search(patterns[problem], output)

    if match is None:
        raise ComparisonError(
            f"Cannot find {problem} result in project output."
        )

    return int(match.group(1))


def read_project_result(
        executable: Path,
        case: Case,
        source: int,
        target: int,
) -> int:
    """
    Uruchamia algorytm projektu jeden raz i odczytuje wynik.

    Używana jest lista sąsiedztwa, ponieważ jej wypisanie
    jest znacznie mniejsze od dużej macierzy incydencji.
    """

    command = command_for_case(
        executable,
        case,
        structure_id=2,
        source=source,
        target=target,
    )

    process = subprocess.run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )

    if process.returncode != 0:
        raise ComparisonError(
            "Project execution failed:\n"
            + " ".join(command)
            + "\n"
            + process.stdout[-3000:]
        )

    return parse_project_result(
        case.problem,
        process.stdout,
    )


def measure_operation(
        operation: Callable[[], int | float],
        iterations: int,
) -> tuple[int | float, float, float, float]:
    """
    Mierzy tylko wywołanie algorytmu NetworkX.

    Graf jest przygotowany przed rozpoczęciem pomiaru.
    """

    times_us: list[float] = []
    expected_result: int | float | None = None

    for _ in range(iterations):
        start_ns = time.perf_counter_ns()
        result = operation()
        end_ns = time.perf_counter_ns()

        if expected_result is None:
            expected_result = result
        elif result != expected_result:
            raise ComparisonError(
                "NetworkX returned different results "
                "between iterations."
            )

        times_us.append(
            (end_ns - start_ns) / 1000.0
        )

    if expected_result is None:
        raise ComparisonError(
            "No NetworkX result was produced."
        )

    return (
        expected_result,
        min(times_us),
        max(times_us),
        sum(times_us) / len(times_us),
    )


def benchmark_networkx(
        mst_graph: nx.Graph,
        shortest_path_graph: nx.DiGraph,
        flow_graph: nx.DiGraph,
        iterations: int,
        source: int,
        target: int,
        skip_mf: bool,
) -> dict[tuple[str, str], dict[str, int | float | str]]:
    results: dict[
        tuple[str, str],
        dict[str, int | float | str],
    ] = {}

    def mst_cost(algorithm: str) -> int:
        tree = nx.minimum_spanning_tree(
            mst_graph,
            algorithm=algorithm,
            weight="weight",
        )

        return int(
            sum(
                data["weight"]
                for _, _, data in tree.edges(data=True)
            )
        )

    operations: list[
        tuple[
            str,
            str,
            str,
            Callable[[], int | float],
        ]
    ] = [
        (
            "MST",
            "Prim",
            "prim",
            lambda: mst_cost("prim"),
        ),
        (
            "MST",
            "Kruskal",
            "kruskal",
            lambda: mst_cost("kruskal"),
        ),
        (
            "SP",
            "Dijkstra",
            "dijkstra_path_length",
            lambda: nx.dijkstra_path_length(
                shortest_path_graph,
                source,
                target,
                weight="weight",
            ),
        ),
        (
            "SP",
            "Bellman-Ford",
            "bellman_ford_path_length",
            lambda: nx.bellman_ford_path_length(
                shortest_path_graph,
                source,
                target,
                weight="weight",
            ),
        ),
    ]

    if not skip_mf:
        operations.append(
            (
                "MF",
                "Ford-Fulkerson",
                "preflow_push",
                lambda: nx.maximum_flow_value(
                    flow_graph,
                    source,
                    target,
                    capacity="capacity",
                    flow_func=preflow_push,
                ),
            )
        )

    for (
            problem,
            project_algorithm,
            networkx_algorithm,
            operation,
    ) in operations:
        print(
            f"NetworkX benchmark: "
            f"{problem} | {networkx_algorithm}"
        )

        (
            result,
            min_time,
            max_time,
            avg_time,
        ) = measure_operation(
            operation,
            iterations,
        )

        results[(problem, project_algorithm)] = {
            "networkxAlgorithm": networkx_algorithm,
            "result": result,
            "minTimeUs": min_time,
            "maxTimeUs": max_time,
            "avgTimeUs": avg_time,
        }

    return results


def read_project_times(
        project_csv: Path,
) -> dict[tuple[str, str, str], dict[str, str]]:
    rows: dict[
        tuple[str, str, str],
        dict[str, str],
    ] = {}

    with project_csv.open(
            "r",
            encoding="utf-8",
            newline="",
    ) as file:
        reader = csv.DictReader(
            file,
            delimiter=";",
        )

        required = {
            "problem",
            "algorithm",
            "structure",
            "vertexCount",
            "edgeCount",
            "iterations",
            "minTimeUs",
            "maxTimeUs",
            "avgTimeUs",
        }

        if reader.fieldnames is None:
            raise ComparisonError(
                f"Missing CSV header: {project_csv}"
            )

        missing = required - set(reader.fieldnames)

        if missing:
            raise ComparisonError(
                "Project CSV is missing columns: "
                + ", ".join(sorted(missing))
            )

        for row in reader:
            key = (
                row["problem"],
                row["algorithm"],
                row["structure"],
            )
            rows[key] = row

    return rows


def write_comparison_csv(
        output_csv: Path,
        project_times: dict[
            tuple[str, str, str],
            dict[str, str],
        ],
        project_results: dict[tuple[str, str], int],
        networkx_results: dict[
            tuple[str, str],
            dict[str, int | float | str],
        ],
        graph_sizes: dict[str, tuple[int, int]],
        iterations: int,
        source: int,
        target: int,
        skip_mf: bool,
) -> bool:
    output_csv.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    fieldnames = [
        "problem",
        "algorithm",
        "implementation",
        "structure",
        "implementationAlgorithm",
        "vertexCount",
        "edgeCount",
        "source",
        "target",
        "result",
        "referenceResult",
        "matchesReference",
        "iterations",
        "minTimeUs",
        "maxTimeUs",
        "avgTimeUs",
    ]

    all_correct = True

    with output_csv.open(
            "w",
            encoding="utf-8",
            newline="",
    ) as file:
        writer = csv.DictWriter(
            file,
            fieldnames=fieldnames,
            delimiter=";",
        )
        writer.writeheader()

        for case in CASES:
            if skip_mf and case.problem == "MF":
                continue

            key = (case.problem, case.algorithm)

            project_result = project_results[key]
            reference_result = networkx_results[key]["result"]
            correct = project_result == reference_result
            all_correct = all_correct and correct

            for structure_name, _ in STRUCTURES:
                time_key = (
                    case.problem,
                    case.algorithm,
                    structure_name,
                )

                if time_key not in project_times:
                    raise ComparisonError(
                        "Missing project benchmark row: "
                        + ";".join(time_key)
                    )

                row = project_times[time_key]

                writer.writerow(
                    {
                        "problem": case.problem,
                        "algorithm": case.algorithm,
                        "implementation": "Project",
                        "structure": structure_name,
                        "implementationAlgorithm": (
                            case.algorithm
                        ),
                        "vertexCount": row["vertexCount"],
                        "edgeCount": row["edgeCount"],
                        "source": (
                            ""
                            if case.problem == "MST"
                            else source
                        ),
                        "target": (
                            ""
                            if case.problem == "MST"
                            else target
                        ),
                        "result": project_result,
                        "referenceResult": reference_result,
                        "matchesReference": correct,
                        "iterations": row["iterations"],
                        "minTimeUs": row["minTimeUs"],
                        "maxTimeUs": row["maxTimeUs"],
                        "avgTimeUs": row["avgTimeUs"],
                    }
                )

            vertex_count, edge_count = graph_sizes[
                case.problem
            ]

            nx_row = networkx_results[key]

            writer.writerow(
                {
                    "problem": case.problem,
                    "algorithm": case.algorithm,
                    "implementation": "NetworkX",
                    "structure": "NetworkXGraph",
                    "implementationAlgorithm": (
                        nx_row["networkxAlgorithm"]
                    ),
                    "vertexCount": vertex_count,
                    "edgeCount": edge_count,
                    "source": (
                        ""
                        if case.problem == "MST"
                        else source
                    ),
                    "target": (
                        ""
                        if case.problem == "MST"
                        else target
                    ),
                    "result": reference_result,
                    "referenceResult": reference_result,
                    "matchesReference": True,
                    "iterations": iterations,
                    "minTimeUs": (
                        f"{float(nx_row['minTimeUs']):.3f}"
                    ),
                    "maxTimeUs": (
                        f"{float(nx_row['maxTimeUs']):.3f}"
                    ),
                    "avgTimeUs": (
                        f"{float(nx_row['avgTimeUs']):.3f}"
                    ),
                }
            )

    return all_correct


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Compare the graph project with NetworkX "
            "on the email-Eu-core dataset."
        )
    )

    parser.add_argument(
        "--executable",
        type=Path,
        default=DEFAULT_EXECUTABLE,
    )

    parser.add_argument(
        "--project-csv",
        type=Path,
        default=DEFAULT_PROJECT_CSV,
    )

    parser.add_argument(
        "--comparison-csv",
        type=Path,
        default=DEFAULT_COMPARISON_CSV,
    )

    parser.add_argument(
        "--iterations",
        type=int,
        default=50,
    )

    parser.add_argument(
        "--source",
        type=int,
        default=0,
    )

    parser.add_argument(
        "--target",
        type=int,
        default=999,
    )

    parser.add_argument(
        "--skip-mf",
        action="store_true",
    )

    args = parser.parse_args()

    if args.iterations <= 0:
        raise ComparisonError(
            "Iterations must be greater than 0."
        )

    check_file(
        args.executable,
        "project executable",
    )
    check_file(
        MST_DATASET,
        "connected MST dataset",
    )
    check_file(
        DIRECTED_DATASET,
        "directed dataset",
    )

    (
        mst_vertex_count,
        mst_edge_count,
        mst_edges,
    ) = read_graph_file(MST_DATASET)

    (
        directed_vertex_count,
        directed_edge_count,
        directed_edges,
    ) = read_graph_file(DIRECTED_DATASET)

    if not (
            0 <= args.source < directed_vertex_count
            and 0 <= args.target < directed_vertex_count
    ):
        raise ComparisonError(
            "Source and target are outside the graph."
        )

    if args.source == args.target:
        raise ComparisonError(
            "Source and target must be different."
        )

    print("Building NetworkX graphs...")

    mst_graph = build_mst_graph(
        mst_vertex_count,
        mst_edges,
    )

    shortest_path_graph = build_shortest_path_graph(
        directed_vertex_count,
        directed_edges,
    )

    flow_graph = build_flow_graph(
        directed_vertex_count,
        directed_edges,
    )

    if not nx.has_path(
            shortest_path_graph,
            args.source,
            args.target,
    ):
        raise ComparisonError(
            f"No directed path from {args.source} "
            f"to {args.target}."
        )

    run_project_benchmarks(
        args.executable,
        args.project_csv,
        args.iterations,
        args.source,
        args.target,
        args.skip_mf,
    )

    project_results: dict[tuple[str, str], int] = {}

    for case in CASES:
        if args.skip_mf and case.problem == "MF":
            continue

        print(
            f"Reading project result: "
            f"{case.problem} | {case.algorithm}"
        )

        project_results[
            (case.problem, case.algorithm)
        ] = read_project_result(
            args.executable,
            case,
            args.source,
            args.target,
        )

    networkx_results = benchmark_networkx(
        mst_graph,
        shortest_path_graph,
        flow_graph,
        args.iterations,
        args.source,
        args.target,
        args.skip_mf,
    )

    print()
    print("Correctness:")

    for case in CASES:
        if args.skip_mf and case.problem == "MF":
            continue

        key = (case.problem, case.algorithm)
        project_value = project_results[key]
        networkx_value = networkx_results[key]["result"]

        print(
            f"  {case.problem} | {case.algorithm}: "
            f"project={project_value}, "
            f"NetworkX={networkx_value}, "
            f"match={project_value == networkx_value}"
        )

    project_times = read_project_times(
        args.project_csv
    )

    all_correct = write_comparison_csv(
        args.comparison_csv,
        project_times,
        project_results,
        networkx_results,
        {
            "MST": (
                mst_vertex_count,
                mst_edge_count,
            ),
            "SP": (
                directed_vertex_count,
                directed_edge_count,
            ),
            "MF": (
                directed_vertex_count,
                directed_edge_count,
            ),
        },
        args.iterations,
        args.source,
        args.target,
        args.skip_mf,
    )

    print()
    print(
        f"Project times: {args.project_csv}"
    )
    print(
        f"Comparison: {args.comparison_csv}"
    )

    if not all_correct:
        raise ComparisonError(
            "At least one project result differs "
            "from the NetworkX reference."
        )

    print("All project results match NetworkX.")


if __name__ == "__main__":
    try:
        main()
    except (
            ComparisonError,
            subprocess.CalledProcessError,
            nx.NetworkXException,
    ) as error:
        print(f"Error: {error}")
        raise SystemExit(1) from error
