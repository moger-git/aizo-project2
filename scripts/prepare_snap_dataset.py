from pathlib import Path


INPUT_FILE = Path(
    "data/datasets/email-Eu-core.txt"
)

OUTPUT_DIRECTED = Path(
    "data/datasets/email_Eu_core_directed_weighted.txt"
)

OUTPUT_MST_CONNECTED = Path(
    "data/datasets/email_Eu_core_mst_connected_weighted.txt"
)

OUTPUT_MST_MAPPING = Path(
    "data/datasets/email_Eu_core_mst_vertex_mapping.csv"
)


def make_weight(u: int, v: int) -> int:
    """
    Tworzy deterministyczną wagę krawędzi.

    Zakres wag wynosi 1-100. Te same dane wejściowe
    zawsze otrzymają te same wagi.
    """

    return 1 + ((u + v) % 100)


def read_directed_edges(
        path: Path,
) -> tuple[int, list[tuple[int, int, int]]]:

    edges: list[tuple[int, int, int]] = []
    max_vertex = -1

    with path.open("r", encoding="utf-8") as file:
        for line_number, line in enumerate(file, start=1):
            line = line.strip()

            if not line or line.startswith("#"):
                continue

            parts = line.split()

            if len(parts) < 2:
                print(
                    f"Warning: skipped incomplete line "
                    f"{line_number}."
                )
                continue

            u = int(parts[0])
            v = int(parts[1])

            if u < 0 or v < 0:
                print(
                    f"Warning: skipped negative vertex "
                    f"at line {line_number}."
                )
                continue

            if u == v:
                # Pętla własna nie jest potrzebna
                # w badanych problemach.
                continue

            weight = make_weight(u, v)

            edges.append((u, v, weight))

            max_vertex = max(
                max_vertex,
                u,
                v,
            )

    return max_vertex + 1, edges


def make_undirected_edges(
        directed_edges: list[tuple[int, int, int]],
) -> list[tuple[int, int, int]]:

    best_edges: dict[tuple[int, int], int] = {}

    for u, v, weight in directed_edges:
        first = min(u, v)
        second = max(u, v)

        key = (first, second)

        if (
                key not in best_edges
                or weight < best_edges[key]
        ):
            best_edges[key] = weight

    result: list[tuple[int, int, int]] = []

    for (u, v), weight in best_edges.items():
        result.append((u, v, weight))

    result.sort()

    return result


def build_undirected_adjacency(
        vertex_count: int,
        edges: list[tuple[int, int, int]],
) -> list[list[int]]:

    adjacency: list[list[int]] = [
        [] for _ in range(vertex_count)
    ]

    for u, v, _ in edges:
        adjacency[u].append(v)
        adjacency[v].append(u)

    return adjacency


def find_connected_components(
        vertex_count: int,
        edges: list[tuple[int, int, int]],
) -> list[list[int]]:

    adjacency = build_undirected_adjacency(
        vertex_count,
        edges,
    )

    visited = [False] * vertex_count
    components: list[list[int]] = []

    for start_vertex in range(vertex_count):
        if visited[start_vertex]:
            continue

        component: list[int] = []
        stack = [start_vertex]

        visited[start_vertex] = True

        while stack:
            vertex = stack.pop()
            component.append(vertex)

            for neighbor in adjacency[vertex]:
                if not visited[neighbor]:
                    visited[neighbor] = True
                    stack.append(neighbor)

        components.append(component)

    return components


def create_connected_mst_dataset(
        vertex_count: int,
        undirected_edges: list[tuple[int, int, int]],
) -> tuple[
    list[tuple[int, int, int]],
    dict[int, int],
    int,
]:

    components = find_connected_components(
        vertex_count,
        undirected_edges,
    )

    largest_component = max(
        components,
        key=len,
    )

    # Sortowanie zapewnia powtarzalne mapowanie.
    largest_component.sort()

    old_to_new: dict[int, int] = {}

    for new_vertex, old_vertex in enumerate(
            largest_component
    ):
        old_to_new[old_vertex] = new_vertex

    component_vertices = set(largest_component)

    connected_edges: list[tuple[int, int, int]] = []

    for old_u, old_v, weight in undirected_edges:
        if (
                old_u not in component_vertices
                or old_v not in component_vertices
        ):
            continue

        new_u = old_to_new[old_u]
        new_v = old_to_new[old_v]

        connected_edges.append(
            (new_u, new_v, weight)
        )

    connected_edges.sort()

    return (
        connected_edges,
        old_to_new,
        len(components),
    )


def write_project_file(
        path: Path,
        vertex_count: int,
        edges: list[tuple[int, int, int]],
) -> None:

    path.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    with path.open("w", encoding="utf-8") as file:
        file.write(
            f"{vertex_count}\t{len(edges)}\n"
        )

        for u, v, weight in edges:
            file.write(
                f"{u}\t{v}\t{weight}\n"
            )


def write_vertex_mapping(
        path: Path,
        old_to_new: dict[int, int],
) -> None:

    path.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    with path.open("w", encoding="utf-8") as file:
        file.write(
            "originalVertex;projectVertex\n"
        )

        for old_vertex in sorted(old_to_new):
            new_vertex = old_to_new[old_vertex]

            file.write(
                f"{old_vertex};{new_vertex}\n"
            )

def verify_connected_graph(
        vertex_count: int,
        edges: list[tuple[int, int, int]],
) -> bool:

    if vertex_count == 0:
        return False

    adjacency = build_undirected_adjacency(
        vertex_count,
        edges,
    )

    visited = [False] * vertex_count
    stack = [0]

    visited[0] = True
    visited_count = 0

    while stack:
        vertex = stack.pop()
        visited_count += 1

        for neighbor in adjacency[vertex]:
            if not visited[neighbor]:
                visited[neighbor] = True
                stack.append(neighbor)

    return visited_count == vertex_count


def main() -> None:
    if not INPUT_FILE.exists():
        raise FileNotFoundError(
            f"Missing input dataset: {INPUT_FILE}"
        )

    (
        original_vertex_count,
        directed_edges,
    ) = read_directed_edges(INPUT_FILE)

    undirected_edges = make_undirected_edges(
        directed_edges
    )

    (
        connected_mst_edges,
        old_to_new,
        component_count,
    ) = create_connected_mst_dataset(
        original_vertex_count,
        undirected_edges,
    )

    connected_vertex_count = len(old_to_new)

    if not verify_connected_graph(
            connected_vertex_count,
            connected_mst_edges,
    ):
        raise RuntimeError(
            "Generated MST dataset is not connected."
        )

    write_project_file(
        OUTPUT_DIRECTED,
        original_vertex_count,
        directed_edges,
    )

    write_project_file(
        OUTPUT_MST_CONNECTED,
        connected_vertex_count,
        connected_mst_edges,
    )

    write_vertex_mapping(
        OUTPUT_MST_MAPPING,
        old_to_new,
    )

    print()
    print("Prepared SNAP email-Eu-core dataset")
    print("===================================")
    print(
        f"Original vertices: "
        f"{original_vertex_count}"
    )
    print(
        f"Directed edges: "
        f"{len(directed_edges)}"
    )
    print(
        f"Unique undirected edges: "
        f"{len(undirected_edges)}"
    )
    print(
        f"Connected components: "
        f"{component_count}"
    )
    print(
        f"Largest component vertices: "
        f"{connected_vertex_count}"
    )
    print(
        f"Largest component edges: "
        f"{len(connected_mst_edges)}"
    )
    print()
    print(
        f"Directed dataset: "
        f"{OUTPUT_DIRECTED}"
    )
    print(
        f"Connected MST dataset: "
        f"{OUTPUT_MST_CONNECTED}"
    )
    print(
        f"Vertex mapping: "
        f"{OUTPUT_MST_MAPPING}"
    )
    print(
        f"Largest component edges: "
        f"{len(connected_mst_edges)}"
    )
    print("MST connectivity verification: correct")



if __name__ == "__main__":
    main()