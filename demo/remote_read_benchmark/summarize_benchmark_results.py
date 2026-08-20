#!/usr/bin/env python3

"""
Combine benchmark JSON files into markdown summary tables for GitHub Actions.
"""

import argparse
import csv
import json
from pathlib import Path
from typing import Any, Dict, Iterable, List, Tuple


def collect_results(json_files: Iterable[Path]) -> List[Dict[str, Any]]:
    results: List[Dict[str, Any]] = []
    for json_file in sorted(json_files):
        payload = json.loads(json_file.read_text(encoding="utf-8"))
        results.extend(payload["results"])
    return results


def render_table(headers: List[str], rows: List[List[str]]) -> str:
    lines = [
        "| " + " | ".join(headers) + " |",
        "| " + " | ".join(["---"] * len(headers)) + " |",
    ]
    lines.extend("| " + " | ".join(row) + " |" for row in rows)
    return "\n".join(lines)


def format_seconds(value: float) -> str:
    return f"{value:.6f}"


def get_fastest_results(results: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    fastest: Dict[Tuple[str, str, str], Dict[str, Any]] = {}
    for result in results:
        key = (result["hdf5_version"], result["implementation"], result["actual_driver"])
        current = fastest.get(key)
        if current is None or result["timings_seconds"]["total"] < current["timings_seconds"]["total"]:
            fastest[key] = result
    return [
        fastest[key]
        for key in sorted(fastest, key=lambda item: (item[0], item[1], item[2]))
    ]


def build_rows(results: List[Dict[str, Any]]) -> List[List[str]]:
    return [
        [
            result["hdf5_version"],
            result["implementation"],
            result["actual_driver"],
            str(result["iteration"]),
            format_seconds(result["timings_seconds"]["read_io"]),
            format_seconds(result["timings_seconds"]["read_nwbfile"]),
            format_seconds(result["timings_seconds"]["find_object"]),
            format_seconds(result["timings_seconds"]["read_slice"]),
            format_seconds(result["timings_seconds"]["total"]),
            str(result["data_size_elements"]),
        ]
        for result in results
    ]


def write_csv(output_path: Path, headers: List[str], rows: List[List[str]]) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", encoding="utf-8", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(headers)
        writer.writerows(rows)


def build_markdown(results: List[Dict[str, Any]]) -> str:
    sorted_results = sorted(
        results,
        key=lambda result: (
            result["hdf5_version"],
            result["implementation"],
            result["actual_driver"],
            result["iteration"],
        ),
    )
    all_rows = build_rows(sorted_results)
    fastest_rows = build_rows(get_fastest_results(sorted_results))

    headers = [
        "HDF5",
        "Implementation",
        "Driver",
        "Run",
        "read_io (s)",
        "read_nwbfile (s)",
        "find_object (s)",
        "read_slice (s)",
        "total (s)",
        "data size",
    ]

    sections = [
        "# Remote Read Benchmark Summary",
        "",
        "## All runs",
        "",
        render_table(headers, all_rows),
        "",
        "## Fastest run per case",
        "",
        render_table(headers, fastest_rows),
        "",
    ]
    return "\n".join(sections)


def main() -> None:
    parser = argparse.ArgumentParser(description="Summarize remote read benchmark JSON results.")
    parser.add_argument(
        "--results-dir",
        type=Path,
        required=True,
        help="Directory containing benchmark JSON files.",
    )
    parser.add_argument(
        "--output-markdown",
        type=Path,
        required=True,
        help="Path to the generated markdown summary.",
    )
    parser.add_argument(
        "--output-json",
        type=Path,
        help="Optional path to the merged raw JSON results.",
    )
    parser.add_argument(
        "--output-all-runs-csv",
        type=Path,
        help="Optional CSV path for the full results table.",
    )
    parser.add_argument(
        "--output-fastest-runs-csv",
        type=Path,
        help="Optional CSV path for the fastest-runs table.",
    )
    args = parser.parse_args()

    json_files = sorted(args.results_dir.glob("benchmark-results-*.json"))
    if not json_files:
        raise FileNotFoundError(f"No benchmark result files found in {args.results_dir}")

    results = collect_results(json_files)
    sorted_results = sorted(
        results,
        key=lambda result: (
            result["hdf5_version"],
            result["implementation"],
            result["actual_driver"],
            result["iteration"],
        ),
    )
    headers = [
        "HDF5",
        "Implementation",
        "Driver",
        "Run",
        "read_io (s)",
        "read_nwbfile (s)",
        "find_object (s)",
        "read_slice (s)",
        "total (s)",
        "data size",
    ]
    markdown = build_markdown(results)

    args.output_markdown.parent.mkdir(parents=True, exist_ok=True)
    args.output_markdown.write_text(markdown, encoding="utf-8")

    if args.output_json is not None:
        args.output_json.parent.mkdir(parents=True, exist_ok=True)
        args.output_json.write_text(json.dumps({"results": results}, indent=2) + "\n", encoding="utf-8")

    if args.output_all_runs_csv is not None:
        write_csv(args.output_all_runs_csv, headers, build_rows(sorted_results))

    if args.output_fastest_runs_csv is not None:
        write_csv(args.output_fastest_runs_csv, headers, build_rows(get_fastest_results(sorted_results)))


if __name__ == "__main__":
    main()
