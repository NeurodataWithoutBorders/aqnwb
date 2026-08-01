#!/usr/bin/env python3

"""
Run the remote read benchmark repeatedly for all C++/Python and ROS3/remfile cases
within a single HDF5 environment and write the raw results to JSON.
"""

import argparse
import json
import subprocess
from pathlib import Path
from typing import Any, Dict, List


def build_case_commands(args: argparse.Namespace) -> List[Dict[str, Any]]:
    return [
        {
            "implementation": "cpp",
            "driver": "ros3",
            "command": [
                str(args.cpp_binary),
                args.s3_path,
                args.aws_region,
                args.object_name,
                args.start_indices,
                args.count_indices,
                "ros3",
                "--json",
            ],
        },
        {
            "implementation": "cpp",
            "driver": "remfile",
            "command": [
                str(args.cpp_binary),
                args.s3_path,
                args.aws_region,
                args.object_name,
                args.start_indices,
                args.count_indices,
                "remfile",
                "--json",
            ],
        },
        {
            "implementation": "python",
            "driver": "ros3",
            "command": [
                args.python_executable,
                str(args.python_script),
                args.s3_path,
                args.aws_region,
                args.object_name,
                args.start_indices,
                args.count_indices,
                "--driver",
                "ros3",
                "--strict-driver",
                "--output-format",
                "json",
            ],
        },
        {
            "implementation": "python",
            "driver": "remfile",
            "command": [
                args.python_executable,
                str(args.python_script),
                args.s3_path,
                args.aws_region,
                args.object_name,
                args.start_indices,
                args.count_indices,
                "--driver",
                "remfile",
                "--strict-driver",
                "--output-format",
                "json",
            ],
        },
    ]


def run_case(case: Dict[str, Any], iteration: int, hdf5_version: str) -> Dict[str, Any]:
    completed = subprocess.run(
        case["command"],
        capture_output=True,
        text=True,
        check=False,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            f"Benchmark failed for {case['implementation']}/{case['driver']} run {iteration} "
            f"(exit code {completed.returncode}).\nSTDOUT:\n{completed.stdout}\nSTDERR:\n{completed.stderr}"
        )

    stdout = completed.stdout.strip()
    try:
        payload = json.loads(stdout)
    except json.JSONDecodeError as exc:
        raise RuntimeError(
            f"Benchmark output was not valid JSON for {case['implementation']}/{case['driver']} "
            f"run {iteration}.\nSTDOUT:\n{stdout}\nSTDERR:\n{completed.stderr}"
        ) from exc

    if payload["implementation"] != case["implementation"]:
        raise RuntimeError(
            f"Expected implementation {case['implementation']}, got {payload['implementation']} "
            f"for run {iteration}."
        )
    if payload["requested_driver"] != case["driver"] or payload["actual_driver"] != case["driver"]:
        raise RuntimeError(
            f"Driver mismatch for {case['implementation']}/{case['driver']} run {iteration}: "
            f"requested={payload['requested_driver']} actual={payload['actual_driver']}."
        )

    payload["hdf5_version"] = hdf5_version
    payload["iteration"] = iteration
    return payload


def main() -> None:
    parser = argparse.ArgumentParser(description="Run all remote read benchmark variants repeatedly.")
    parser.add_argument("--cpp-binary", type=Path, required=True, help="Path to the C++ benchmark executable.")
    parser.add_argument("--python-script", type=Path, required=True, help="Path to benchmark.py.")
    parser.add_argument("--python-executable", default="python", help="Python executable used to run benchmark.py.")
    parser.add_argument("--hdf5-version", required=True, help="Label for the HDF5 version under test.")
    parser.add_argument("--repetitions", type=int, default=10, help="Number of repetitions per case.")
    parser.add_argument("--output-json", type=Path, required=True, help="Path to the output JSON file.")
    parser.add_argument("s3_path", help="Remote NWB file URL.")
    parser.add_argument("aws_region", help="AWS region for ROS3.")
    parser.add_argument("object_name", help="Name of the NWB object to read.")
    parser.add_argument("start_indices", help="Comma-separated slice start indices.")
    parser.add_argument("count_indices", help="Comma-separated slice counts.")
    args = parser.parse_args()

    if args.repetitions < 1:
        raise ValueError("--repetitions must be at least 1")

    cases = build_case_commands(args)
    results = []
    for case in cases:
        for iteration in range(1, args.repetitions + 1):
            print(
                f"Running HDF5 {args.hdf5_version}: {case['implementation']}/{case['driver']} "
                f"iteration {iteration}/{args.repetitions}",
                flush=True,
            )
            results.append(run_case(case, iteration, args.hdf5_version))

    output_payload = {
        "benchmark_suite": "remote_read_benchmark",
        "hdf5_version": args.hdf5_version,
        "repetitions": args.repetitions,
        "target": {
            "s3_path": args.s3_path,
            "aws_region": args.aws_region,
            "object_name": args.object_name,
            "start_indices": args.start_indices,
            "count_indices": args.count_indices,
        },
        "results": results,
    }

    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(output_payload, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
