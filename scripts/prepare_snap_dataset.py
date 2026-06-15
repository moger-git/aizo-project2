from pathlib import Path


INPUT_FILE = Path("data/datasets/email-Eu-core.txt")
OUTPUT_SP_MF = Path("data/datasets/email_Eu_core_directed_weighted.txt")
OUTPUT_MST = Path("data/datasets/email_Eu_core_undirected_weighted.txt")


def make_weight(u: int, v: int) -> int:
    return 1 + ((u + v) % 100)


def read_edges(path: Path):
    edges = []
    max_node = -1

    with path.open("r", encoding="utf-8") as file:
        for line in file:
            line = line.strip()

            if not line or line.startswith("#"):
                continue

            parts = line.split()

            if len(parts) < 2:
                continue

            u = int(parts[0])
            v = int(parts[1])

            if u == v:
                continue

            edges.append((u, v, make_weight(u, v)))
            max_node = max(max_node, u, v)

    return max_node + 1, edges


def write_project_file(path: Path, vertex_count: int, edges):
    with path.open("w", encoding="utf-8") as file:
        file.write(f"{vertex_count}\t{len(edges)}\n")

        for u, v, w in edges:
            file.write(f"{u}\t{v}\t{w}\n")


def make_undirected_edges(edges):
    best_edges = {}

    for u, v, w in edges:
        a = min(u, v)
        b = max(u, v)

        key = (a, b)

        if key not in best_edges or w < best_edges[key]:
            best_edges[key] = w

    result = []

    for (u, v), w in best_edges.items():
        result.append((u, v, w))

    return result


def main():
    vertex_count, directed_edges = read_edges(INPUT_FILE)

    undirected_edges = make_undirected_edges(directed_edges)

    write_project_file(OUTPUT_SP_MF, vertex_count, directed_edges)
    write_project_file(OUTPUT_MST, vertex_count, undirected_edges)

    print("Prepared SNAP dataset:")
    print(f"Directed file:   {OUTPUT_SP_MF}")
    print(f"Undirected file: {OUTPUT_MST}")
    print(f"Vertices: {vertex_count}")
    print(f"Directed edges: {len(directed_edges)}")
    print(f"Undirected edges: {len(undirected_edges)}")


if __name__ == "__main__":
    main()