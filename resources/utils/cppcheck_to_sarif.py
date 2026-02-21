#!/usr/bin/env python3
# /// script
# requires-python = ">=3.8"
# ///
"""Convert cppcheck XML output (version 2) to SARIF 2.1.0 format."""

import argparse
import json
import xml.etree.ElementTree as ET
from pathlib import Path


def severity_to_sarif_level(severity: str) -> str:
    mapping = {
        "error": "error",
        "warning": "warning",
        "style": "note",
        "performance": "note",
        "portability": "note",
        "information": "none",
    }
    return mapping.get(severity, "note")


def convert(xml_file: str, sarif_file: str) -> None:
    tree = ET.parse(xml_file)
    root = tree.getroot()

    cppcheck_elem = root.find("cppcheck")
    cppcheck_version = (
        cppcheck_elem.get("version", "unknown")
        if cppcheck_elem is not None
        else "unknown"
    )

    rules: dict = {}
    results = []

    errors_elem = root.find("errors")
    if errors_elem is not None:
        for error in errors_elem.findall("error"):
            rule_id = error.get("id", "unknown")
            severity = error.get("severity", "information")
            msg = error.get("msg", "")
            verbose = error.get("verbose", msg)
            cwe = error.get("cwe")

            if rule_id not in rules:
                rule: dict = {
                    "id": rule_id,
                    "shortDescription": {"text": msg},
                    "fullDescription": {"text": verbose},
                    "defaultConfiguration": {
                        "level": severity_to_sarif_level(severity)
                    },
                    "properties": {"tags": [severity]},
                }
                if cwe:
                    rule["relationships"] = [
                        {
                            "target": {
                                "id": f"CWE-{cwe}",
                                "toolComponent": {"name": "CWE"},
                            }
                        }
                    ]
                rules[rule_id] = rule

            locations = []
            for location in error.findall("location"):
                file_path = location.get("file", "")
                line = int(location.get("line", 1) or 1)
                column = int(location.get("column", 1) or 1)
                locations.append(
                    {
                        "physicalLocation": {
                            "artifactLocation": {
                                "uri": file_path.replace("\\", "/"),
                                "uriBaseId": "%SRCROOT%",
                            },
                            "region": {
                                "startLine": max(line, 1),
                                "startColumn": max(column, 1),
                            },
                        }
                    }
                )

            if not locations:
                continue

            result: dict = {
                "ruleId": rule_id,
                "level": severity_to_sarif_level(severity),
                "message": {"text": verbose},
                "locations": [locations[0]],
            }
            if len(locations) > 1:
                result["relatedLocations"] = [
                    {
                        "id": i,
                        "message": {"text": "Related location"},
                        "physicalLocation": loc["physicalLocation"],
                    }
                    for i, loc in enumerate(locations[1:], 1)
                ]
            results.append(result)

    sarif = {
        "$schema": "https://json.schemastore.org/sarif-2.1.0.json",
        "version": "2.1.0",
        "runs": [
            {
                "tool": {
                    "driver": {
                        "name": "Cppcheck",
                        "version": cppcheck_version,
                        "informationUri": "https://cppcheck.sourceforge.io/",
                        "rules": list(rules.values()),
                    }
                },
                "results": results,
            }
        ],
    }

    Path(sarif_file).write_text(json.dumps(sarif, indent=2))
    print(f"Converted {len(results)} result(s) to {sarif_file}")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Convert cppcheck XML output to SARIF 2.1.0 format"
    )
    parser.add_argument("xml_file", help="Input cppcheck XML file (version 2)")
    parser.add_argument("sarif_file", help="Output SARIF file")
    args = parser.parse_args()
    convert(args.xml_file, args.sarif_file)


if __name__ == "__main__":
    main()
