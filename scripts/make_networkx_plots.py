from __future__ import annotations

import argparse
import csv
import math
import re
from dataclasses import dataclass
from pathlib import Path

import matplotlib.pyplot as plt


DEFAULT_INPUT = Path(
    "results/email_dataset/comparison_networkx.csv"
)

DEFAULT_OUTPUT_DIR = Path(
    "results/email_dataset/plots_networkx"
)


@dataclass(frozen=True)
class ResultRow:
    problem: str
    algorithm: str
    implementation: str
    structure: str
    implementation_algorithm: str
    vertex_count: int
    edge_count: int
    result: str
    reference_result: str
    matches_reference: bool
    iterations: int
    min_time_us: float
    max_time_us: float
    avg_time_us: float

    @property
    def label(self) -> str:
        if self.implementation == "Project":
            if self.structure == "IncidenceMatrix":
                return "Projekt – macierz"
            if self.structure == "AdjacencyList":
                return "Projekt – lista"

            return f"Projekt – {self.structure}"

        return "NetworkX"


def parse_bool(value: str) -> bool:
    normalized = value.strip().lower()

    if normalized in {"true", "1", "yes"}:
        return True

    if normalized in {"false", "0", "no"}:
        return False

    raise ValueError(f"Niepoprawna wartość logiczna: {value}")


def parse_float(value: str, column: str) -> float:
    try:
        return float(value.replace(",", "."))
    except ValueError as error:
        raise ValueError(
            f"Niepoprawna liczba w kolumnie {column}: {value}"
        ) from error


def read_results(path: Path) -> list[ResultRow]:
    if not path.is_file():
        raise FileNotFoundError(
            f"Nie znaleziono pliku CSV: {path}"
        )

    with path.open(
            "r",
            encoding="utf-8",
            newline="",
    ) as file:
        reader = csv.DictReader(
            file,
            delimiter=";",
        )

        required_columns = {
            "problem",
            "algorithm",
            "implementation",
            "structure",
            "implementationAlgorithm",
            "vertexCount",
            "edgeCount",
            "result",
            "referenceResult",
            "matchesReference",
            "iterations",
            "minTimeUs",
            "maxTimeUs",
            "avgTimeUs",
        }

        if reader.fieldnames is None:
            raise ValueError(
                "Plik CSV nie zawiera nagłówka."
            )

        missing_columns = (
                required_columns - set(reader.fieldnames)
        )

        if missing_columns:
            raise ValueError(
                "Brakujące kolumny w CSV: "
                + ", ".join(sorted(missing_columns))
            )

        rows: list[ResultRow] = []

        for line_number, row in enumerate(
                reader,
                start=2,
        ):
            try:
                result_row = ResultRow(
                    problem=row["problem"].strip(),
                    algorithm=row["algorithm"].strip(),
                    implementation=(
                        row["implementation"].strip()
                    ),
                    structure=row["structure"].strip(),
                    implementation_algorithm=(
                        row["implementationAlgorithm"].strip()
                    ),
                    vertex_count=int(row["vertexCount"]),
                    edge_count=int(row["edgeCount"]),
                    result=row["result"].strip(),
                    reference_result=(
                        row["referenceResult"].strip()
                    ),
                    matches_reference=parse_bool(
                        row["matchesReference"]
                    ),
                    iterations=int(row["iterations"]),
                    min_time_us=parse_float(
                        row["minTimeUs"],
                        "minTimeUs",
                    ),
                    max_time_us=parse_float(
                        row["maxTimeUs"],
                        "maxTimeUs",
                    ),
                    avg_time_us=parse_float(
                        row["avgTimeUs"],
                        "avgTimeUs",
                    ),
                )
            except (ValueError, KeyError) as error:
                raise ValueError(
                    f"Błąd w wierszu {line_number}: {error}"
                ) from error

            if result_row.avg_time_us < 0:
                raise ValueError(
                    f"Ujemny czas w wierszu {line_number}."
                )

            rows.append(result_row)

    if not rows:
        raise ValueError(
            "Plik CSV nie zawiera wyników."
        )

    return rows


def safe_filename(value: str) -> str:
    normalized = value.lower().replace("ł", "l")
    normalized = normalized.replace("–", "-")
    normalized = re.sub(
        r"[^a-z0-9]+",
        "_",
        normalized,
    )

    return normalized.strip("_")


def group_by_case(
        rows: list[ResultRow],
) -> dict[tuple[str, str], list[ResultRow]]:
    grouped: dict[
        tuple[str, str],
        list[ResultRow],
    ] = {}

    for row in rows:
        key = (row.problem, row.algorithm)
        grouped.setdefault(key, []).append(row)

    return grouped


def ordered_case_keys(
        grouped: dict[
            tuple[str, str],
            list[ResultRow],
        ],
) -> list[tuple[str, str]]:
    preferred_order = [
        ("MST", "Prim"),
        ("MST", "Kruskal"),
        ("SP", "Dijkstra"),
        ("SP", "Bellman-Ford"),
        ("MF", "Ford-Fulkerson"),
    ]

    ordered = [
        key
        for key in preferred_order
        if key in grouped
    ]

    remaining = sorted(
        key
        for key in grouped
        if key not in ordered
    )

    return ordered + remaining


def ordered_rows(
        rows: list[ResultRow],
) -> list[ResultRow]:
    order = {
        ("Project", "IncidenceMatrix"): 0,
        ("Project", "AdjacencyList"): 1,
        ("NetworkX", "NetworkXGraph"): 2,
    }

    return sorted(
        rows,
        key=lambda row: order.get(
            (row.implementation, row.structure),
            100,
        ),
    )


def should_use_log_scale(
        values: list[float],
        force_log: bool,
) -> bool:
    if force_log:
        return True

    positive_values = [
        value
        for value in values
        if value > 0
    ]

    if len(positive_values) < 2:
        return False

    return (
            max(positive_values) /
            min(positive_values)
    ) >= 20.0


def save_figure(
        path: Path,
        dpi: int,
) -> None:
    path.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    plt.tight_layout()
    plt.savefig(
        path,
        dpi=dpi,
        bbox_inches="tight",
    )
    plt.close()

    print(f"Zapisano: {path}")


def plot_single_case(
        problem: str,
        algorithm: str,
        rows: list[ResultRow],
        output_dir: Path,
        dpi: int,
        force_log: bool,
) -> None:
    sorted_rows = ordered_rows(rows)

    labels = [
        row.label
        for row in sorted_rows
    ]

    averages = [
        row.avg_time_us
        for row in sorted_rows
    ]

    lower_errors = [
        max(
            0.0,
            row.avg_time_us - row.min_time_us,
            )
        for row in sorted_rows
    ]

    upper_errors = [
        max(
            0.0,
            row.max_time_us - row.avg_time_us,
            )
        for row in sorted_rows
    ]

    positions = list(range(len(sorted_rows)))

    plt.figure(
        figsize=(9, 6),
    )

    plt.bar(
        positions,
        averages,
        yerr=[lower_errors, upper_errors],
        capsize=6,
    )

    plt.xticks(
        positions,
        labels,
        rotation=12,
        ha="right",
    )

    plt.ylabel(
        "Średni czas [µs]"
    )

    plt.title(
        f"{problem} – {algorithm}\n"
        "Projekt a NetworkX"
    )

    plt.grid(
        axis="y",
        linestyle="--",
        alpha=0.4,
    )

    if should_use_log_scale(
            averages,
            force_log,
    ):
        plt.yscale("log")

    for position, value in zip(
            positions,
            averages,
    ):
        plt.text(
            position,
            value,
            f"{value:.2f}",
            ha="center",
            va="bottom",
            fontsize=9,
        )

    filename = (
        f"{safe_filename(problem)}_"
        f"{safe_filename(algorithm)}.png"
    )

    save_figure(
        output_dir / filename,
        dpi,
        )


def plot_all_average_times(
        grouped: dict[
            tuple[str, str],
            list[ResultRow],
        ],
        output_dir: Path,
        dpi: int,
        force_log: bool,
) -> None:
    case_keys = ordered_case_keys(grouped)

    labels = [
        f"{problem}\n{algorithm}"
        for problem, algorithm in case_keys
    ]

    series = [
        (
            "Projekt – macierz",
            "Project",
            "IncidenceMatrix",
        ),
        (
            "Projekt – lista",
            "Project",
            "AdjacencyList",
        ),
        (
            "NetworkX",
            "NetworkX",
            "NetworkXGraph",
        ),
    ]

    width = 0.25
    base_positions = list(
        range(len(case_keys))
    )

    all_values: list[float] = []

    plt.figure(
        figsize=(13, 7),
    )

    for series_index, (
            series_label,
            implementation,
            structure,
    ) in enumerate(series):
        values: list[float] = []

        for key in case_keys:
            matching = [
                row
                for row in grouped[key]
                if (
                        row.implementation == implementation
                        and row.structure == structure
                )
            ]

            if not matching:
                values.append(float("nan"))
            else:
                values.append(
                    matching[0].avg_time_us
                )
                all_values.append(
                    matching[0].avg_time_us
                )

        positions = [
            position
            + (series_index - 1) * width
            for position in base_positions
        ]

        plt.bar(
            positions,
            values,
            width=width,
            label=series_label,
        )

    plt.xticks(
        base_positions,
        labels,
    )

    plt.ylabel(
        "Średni czas [µs]"
    )

    plt.title(
        "Porównanie średnich czasów – projekt i NetworkX"
    )

    plt.legend()

    plt.grid(
        axis="y",
        linestyle="--",
        alpha=0.4,
    )

    if should_use_log_scale(
            all_values,
            force_log,
    ):
        plt.yscale("log")

    save_figure(
        output_dir / "comparison_all_avg.png",
        dpi,
        )


def find_networkx_row(
        rows: list[ResultRow],
) -> ResultRow:
    matching = [
        row
        for row in rows
        if row.implementation == "NetworkX"
    ]

    if len(matching) != 1:
        raise ValueError(
            "Każdy przypadek musi zawierać dokładnie "
            "jeden wynik NetworkX."
        )

    return matching[0]


def find_project_row(
        rows: list[ResultRow],
        structure: str,
) -> ResultRow:
    matching = [
        row
        for row in rows
        if (
                row.implementation == "Project"
                and row.structure == structure
        )
    ]

    if len(matching) != 1:
        raise ValueError(
            "Brak jednoznacznego wyniku projektu dla "
            f"struktury {structure}."
        )

    return matching[0]


def plot_time_ratio(
        grouped: dict[
            tuple[str, str],
            list[ResultRow],
        ],
        output_dir: Path,
        dpi: int,
) -> None:
    case_keys = ordered_case_keys(grouped)

    labels = [
        f"{problem}\n{algorithm}"
        for problem, algorithm in case_keys
    ]

    matrix_ratios: list[float] = []
    list_ratios: list[float] = []

    for key in case_keys:
        rows = grouped[key]
        networkx_row = find_networkx_row(rows)
        matrix_row = find_project_row(
            rows,
            "IncidenceMatrix",
        )
        list_row = find_project_row(
            rows,
            "AdjacencyList",
        )

        if networkx_row.avg_time_us <= 0:
            raise ValueError(
                "Czas NetworkX musi być większy od zera."
            )

        matrix_ratios.append(
            matrix_row.avg_time_us /
            networkx_row.avg_time_us
        )

        list_ratios.append(
            list_row.avg_time_us /
            networkx_row.avg_time_us
        )

    width = 0.35
    base_positions = list(
        range(len(case_keys))
    )

    matrix_positions = [
        position - width / 2
        for position in base_positions
    ]

    list_positions = [
        position + width / 2
        for position in base_positions
    ]

    plt.figure(
        figsize=(13, 7),
    )

    plt.bar(
        matrix_positions,
        matrix_ratios,
        width=width,
        label="Projekt – macierz / NetworkX",
    )

    plt.bar(
        list_positions,
        list_ratios,
        width=width,
        label="Projekt – lista / NetworkX",
    )

    plt.axhline(
        1.0,
        linestyle="--",
        linewidth=1.2,
        label="Taki sam czas jak NetworkX",
    )

    plt.xticks(
        base_positions,
        labels,
    )

    plt.ylabel(
        "Iloraz średnich czasów"
    )

    plt.title(
        "Stosunek czasu projektu do czasu NetworkX"
    )

    plt.legend()

    plt.grid(
        axis="y",
        linestyle="--",
        alpha=0.4,
    )

    positive_ratios = [
        ratio
        for ratio in matrix_ratios + list_ratios
        if ratio > 0
    ]

    if positive_ratios and (
            max(positive_ratios)
            / min(positive_ratios)
            >= 20.0
    ):
        plt.yscale("log")

    for positions, values in (
            (matrix_positions, matrix_ratios),
            (list_positions, list_ratios),
    ):
        for position, value in zip(
                positions,
                values,
        ):
            plt.text(
                position,
                value,
                f"{value:.2f}×",
                ha="center",
                va="bottom",
                fontsize=8,
            )

    save_figure(
        output_dir / "time_ratio_vs_networkx.png",
        dpi,
        )


def write_summary(
        grouped: dict[
            tuple[str, str],
            list[ResultRow],
        ],
        output_path: Path,
) -> None:
    output_path.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    with output_path.open(
            "w",
            encoding="utf-8",
    ) as file:
        file.write(
            "Podsumowanie porównania z NetworkX\n"
        )
        file.write(
            "=================================\n\n"
        )

        for key in ordered_case_keys(grouped):
            problem, algorithm = key
            rows = ordered_rows(grouped[key])

            file.write(
                f"{problem} – {algorithm}\n"
            )

            for row in rows:
                file.write(
                    f"  {row.label}: "
                    f"avg={row.avg_time_us:.3f} µs, "
                    f"min={row.min_time_us:.3f} µs, "
                    f"max={row.max_time_us:.3f} µs, "
                    f"zgodność={row.matches_reference}\n"
                )

            file.write("\n")

    print(f"Zapisano: {output_path}")


def verify_correctness(
        rows: list[ResultRow],
) -> None:
    incorrect = [
        row
        for row in rows
        if not row.matches_reference
    ]

    if not incorrect:
        print(
            "Wszystkie wyniki są zgodne z NetworkX."
        )
        return

    print(
        "Uwaga: znaleziono wyniki niezgodne z NetworkX:"
    )

    for row in incorrect:
        print(
            f"  {row.problem} | {row.algorithm} | "
            f"{row.label}: result={row.result}, "
            f"reference={row.reference_result}"
        )


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Buduje wykresy porównujące czasy projektu "
            "z biblioteką NetworkX."
        )
    )

    parser.add_argument(
        "--input",
        type=Path,
        default=DEFAULT_INPUT,
        help=(
            "Plik comparison_networkx.csv."
        ),
    )

    parser.add_argument(
        "--output-dir",
        type=Path,
        default=DEFAULT_OUTPUT_DIR,
        help=(
            "Katalog, do którego zostaną zapisane wykresy."
        ),
    )

    parser.add_argument(
        "--dpi",
        type=int,
        default=180,
        help="Rozdzielczość obrazów PNG.",
    )

    parser.add_argument(
        "--log",
        action="store_true",
        help=(
            "Wymusza logarytmiczną skalę czasu."
        ),
    )

    args = parser.parse_args()

    if args.dpi <= 0:
        raise ValueError(
            "DPI musi być większe od zera."
        )

    rows = read_results(args.input)
    grouped = group_by_case(rows)

    verify_correctness(rows)

    args.output_dir.mkdir(
        parents=True,
        exist_ok=True,
    )

    for (
            problem,
            algorithm,
    ), case_rows in grouped.items():
        plot_single_case(
            problem,
            algorithm,
            case_rows,
            args.output_dir,
            args.dpi,
            args.log,
        )

    plot_all_average_times(
        grouped,
        args.output_dir,
        args.dpi,
        args.log,
    )

    plot_time_ratio(
        grouped,
        args.output_dir,
        args.dpi,
    )

    write_summary(
        grouped,
        args.output_dir / "summary.txt",
        )

    print()
    print("Tworzenie wykresów zakończone.")
    print(
        f"Katalog wynikowy: {args.output_dir}"
    )


if __name__ == "__main__":
    try:
        main()
    except (
            FileNotFoundError,
            ValueError,
            csv.Error,
    ) as error:
        print(f"Błąd: {error}")
        raise SystemExit(1) from error