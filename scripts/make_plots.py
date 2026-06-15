import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")

import matplotlib.pyplot as plt
import pandas as pd


REQUIRED_COLUMNS = [
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


def read_benchmark_csv(path: Path) -> pd.DataFrame:
    if not path.exists():
        raise FileNotFoundError(f"CSV file does not exist: {path}")

    df = pd.read_csv(path, sep=";")

    missing_columns = [column for column in REQUIRED_COLUMNS if column not in df.columns]

    if missing_columns:
        raise ValueError(
            f"File {path} has invalid format. Missing columns: {missing_columns}"
        )

    numeric_columns = [
        "vertexCount",
        "edgeCount",
        "density",
        "iterations",
        "minTimeUs",
        "maxTimeUs",
        "avgTimeUs",
    ]

    for column in numeric_columns:
        df[column] = (
            df[column]
            .astype(str)
            .str.replace(",", ".", regex=False)
        )
        df[column] = pd.to_numeric(df[column], errors="coerce")

    df = df.dropna(subset=numeric_columns)

    return df


def prepare_grouped_data(df: pd.DataFrame, x_column: str) -> pd.DataFrame:
    grouped = (
        df.groupby(
            ["problem", "algorithm", "structure", x_column],
            as_index=False,
        )
        .agg(
            minTimeUs=("minTimeUs", "min"),
            maxTimeUs=("maxTimeUs", "max"),
            avgTimeUs=("avgTimeUs", "mean"),
            iterations=("iterations", "sum"),
            edgeCount=("edgeCount", "mean"),
        )
    )

    return grouped


def make_safe_filename(text: str) -> str:
    result = text.lower()
    result = result.replace(" ", "_")
    result = result.replace("-", "_")
    result = result.replace("/", "_")
    result = result.replace("\\", "_")
    result = result.replace("|", "_")
    result = result.replace(":", "_")
    return result


def format_time_axis(ax) -> None:
    ax.set_ylabel("Average time [us]")
    ax.grid(True, which="both", axis="both", linestyle="--", linewidth=0.5)


def plot_study(
        df: pd.DataFrame,
        study_name: str,
        x_column: str,
        x_label: str,
        output_dir: Path,
        use_log_scale: bool,
) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)

    grouped = prepare_grouped_data(df, x_column)

    for problem in sorted(grouped["problem"].unique()):
        problem_df = grouped[grouped["problem"] == problem]

        plt.figure(figsize=(11, 7))

        for (algorithm, structure), series_df in problem_df.groupby(
                ["algorithm", "structure"]
        ):
            series_df = series_df.sort_values(x_column)

            label = f"{algorithm} | {structure}"

            plt.plot(
                series_df[x_column],
                series_df["avgTimeUs"],
                marker="o",
                linewidth=2,
                label=label,
            )

        ax = plt.gca()
        format_time_axis(ax)

        if use_log_scale:
            ax.set_yscale("log")

        plt.xlabel(x_label)
        plt.title(f"{study_name}: {problem} - average time")
        plt.legend()
        plt.tight_layout()

        filename = f"{make_safe_filename(study_name)}_{make_safe_filename(problem)}_avg_time.png"
        output_path = output_dir / filename

        plt.savefig(output_path, dpi=200)
        plt.close()

        print(f"Saved: {output_path}")


def plot_min_max_for_study(
        df: pd.DataFrame,
        study_name: str,
        x_column: str,
        x_label: str,
        output_dir: Path,
        use_log_scale: bool,
) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)

    grouped = prepare_grouped_data(df, x_column)

    for problem in sorted(grouped["problem"].unique()):
        problem_df = grouped[grouped["problem"] == problem]

        for algorithm in sorted(problem_df["algorithm"].unique()):
            algorithm_df = problem_df[problem_df["algorithm"] == algorithm]

            plt.figure(figsize=(11, 7))

            for structure, series_df in algorithm_df.groupby("structure"):
                series_df = series_df.sort_values(x_column)

                plt.plot(
                    series_df[x_column],
                    series_df["avgTimeUs"],
                    marker="o",
                    linewidth=2,
                    label=f"{structure} avg",
                )

                plt.fill_between(
                    series_df[x_column],
                    series_df["minTimeUs"],
                    series_df["maxTimeUs"],
                    alpha=0.15,
                    label=f"{structure} min-max",
                )

            ax = plt.gca()
            format_time_axis(ax)

            if use_log_scale:
                ax.set_yscale("log")

            plt.xlabel(x_label)
            plt.title(f"{study_name}: {problem} - {algorithm} min/avg/max")
            plt.legend()
            plt.tight_layout()

            filename = (
                f"{make_safe_filename(study_name)}_"
                f"{make_safe_filename(problem)}_"
                f"{make_safe_filename(algorithm)}_min_avg_max.png"
            )

            output_path = output_dir / filename

            plt.savefig(output_path, dpi=200)
            plt.close()

            print(f"Saved: {output_path}")


def plot_dataset_summary(
        df: pd.DataFrame,
        output_dir: Path,
        use_log_scale: bool,
) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)

    grouped = (
        df.groupby(
            ["problem", "algorithm", "structure"],
            as_index=False,
        )
        .agg(
            avgTimeUs=("avgTimeUs", "mean"),
            minTimeUs=("minTimeUs", "min"),
            maxTimeUs=("maxTimeUs", "max"),
            vertexCount=("vertexCount", "first"),
            edgeCount=("edgeCount", "first"),
        )
    )

    grouped["label"] = (
            grouped["problem"]
            + " | "
            + grouped["algorithm"]
            + " | "
            + grouped["structure"]
    )

    grouped = grouped.sort_values("avgTimeUs", ascending=True)

    plt.figure(figsize=(12, 8))

    plt.barh(grouped["label"], grouped["avgTimeUs"])

    ax = plt.gca()
    ax.set_xlabel("Average time [us]")
    ax.set_ylabel("Algorithm")

    if use_log_scale:
        ax.set_xscale("log")

    plt.title("Dataset email-Eu-core: average time by algorithm")
    plt.tight_layout()

    output_path = output_dir / "dataset_email_avg_time_bar.png"

    plt.savefig(output_path, dpi=200)
    plt.close()

    print(f"Saved: {output_path}")


def save_grouped_summary(
        df: pd.DataFrame,
        output_path: Path,
        x_column: str,
) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    grouped = prepare_grouped_data(df, x_column)
    grouped.to_csv(output_path, sep=";", index=False)

    print(f"Saved summary CSV: {output_path}")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build plots for AiZO Project 2 benchmark results."
    )

    parser.add_argument(
        "--input-a",
        default="results/benchmark_A.csv",
        help="CSV file with study A results.",
    )

    parser.add_argument(
        "--input-b",
        default="results/benchmark_B.csv",
        help="CSV file with study B results.",
    )

    parser.add_argument(
        "--dataset",
        default="results/email_dataset/email_dataset_results.csv",
        help="CSV file with dataset email-Eu-core results.",
    )

    parser.add_argument(
        "--output-dir",
        default="results/plots",
        help="Directory where plots will be saved.",
    )

    parser.add_argument(
        "--log-y",
        action="store_true",
        help="Use logarithmic scale for time axis.",
    )

    parser.add_argument(
        "--skip-dataset",
        action="store_true",
        help="Do not generate dataset plots.",
    )

    args = parser.parse_args()

    output_dir = Path(args.output_dir)

    input_a = Path(args.input_a)
    input_b = Path(args.input_b)
    dataset_input = Path(args.dataset)

    if input_a.exists():
        print(f"Reading study A: {input_a}")
        df_a = read_benchmark_csv(input_a)

        plot_study(
            df=df_a,
            study_name="Study A",
            x_column="vertexCount",
            x_label="Vertex count",
            output_dir=output_dir / "study_A",
            use_log_scale=args.log_y,
        )

        plot_min_max_for_study(
            df=df_a,
            study_name="Study A",
            x_column="vertexCount",
            x_label="Vertex count",
            output_dir=output_dir / "study_A_min_max",
            use_log_scale=args.log_y,
        )

        save_grouped_summary(
            df=df_a,
            output_path=output_dir / "study_A_summary.csv",
            x_column="vertexCount",
        )
    else:
        print(f"Skipped study A. File not found: {input_a}")

    if input_b.exists():
        print(f"Reading study B: {input_b}")
        df_b = read_benchmark_csv(input_b)

        plot_study(
            df=df_b,
            study_name="Study B",
            x_column="density",
            x_label="Density [%]",
            output_dir=output_dir / "study_B",
            use_log_scale=args.log_y,
        )

        plot_min_max_for_study(
            df=df_b,
            study_name="Study B",
            x_column="density",
            x_label="Density [%]",
            output_dir=output_dir / "study_B_min_max",
            use_log_scale=args.log_y,
        )

        save_grouped_summary(
            df=df_b,
            output_path=output_dir / "study_B_summary.csv",
            x_column="density",
        )
    else:
        print(f"Skipped study B. File not found: {input_b}")

    if not args.skip_dataset:
        if dataset_input.exists():
            print(f"Reading dataset results: {dataset_input}")
            df_dataset = read_benchmark_csv(dataset_input)

            plot_dataset_summary(
                df=df_dataset,
                output_dir=output_dir / "dataset",
                use_log_scale=args.log_y,
            )
        else:
            print(f"Skipped dataset plots. File not found: {dataset_input}")

    print()
    print("Finished building plots.")
    print(f"Plots directory: {output_dir}")


if __name__ == "__main__":
    main()