#!/usr/bin/env python3
"""
convert_location.py - normalize a COT / VPP Admin Tools (or any similar
object placement tool) export into the canonical JSON format SM_DungeonMod
expects for its "extra content" (statics / infected / animals) importer:

    { "Objects": [
        { "ClassName": "...", "Position": [x, y, z], "Orientation": [x, y, z] },
        ...
    ]}

COT and VPP Admin Tools are third-party Workshop mods and their exact export
schema can change between versions/updates. Rather than guess and risk a
silent mismatch in-game, run your export through this script once - it
auto-detects the most common key names and array/object vector encodings,
and always writes out the exact shape SM_DungeonMod reads.

Usage:
    python convert_location.py input.json output.json
    python convert_location.py input.json output.json --classname-key Name
    python convert_location.py input.json output.json \
        --classname-key Name --position-key Pos --orientation-key Ori

If auto-detection picks the wrong keys for your export, just tell it the
right ones with --classname-key / --position-key / --orientation-key.
"""

import argparse
import json
import sys

CLASSNAME_KEYS = ["ClassName", "Classname", "className", "Name", "Type", "class", "classname"]
POSITION_KEYS = ["Position", "position", "Pos", "pos", "WorldPosition"]
ORIENTATION_KEYS = ["Orientation", "orientation", "Ori", "ori", "Rotation", "rotation"]
ARRAY_WRAPPER_KEYS = ["Objects", "objects", "Data", "data", "Items", "items", "Entries", "entries"]


def find_key(entry, candidates, override):
    if override:
        return override if override in entry else None
    for key in candidates:
        if key in entry:
            return key
    return None


def to_vector(value):
    if value is None:
        return [0.0, 0.0, 0.0]
    if isinstance(value, dict):
        return [float(value.get("x", 0)), float(value.get("y", 0)), float(value.get("z", 0))]
    if isinstance(value, (list, tuple)):
        parts = [float(v) for v in value]
        while len(parts) < 3:
            parts.append(0.0)
        return parts[:3]
    if isinstance(value, str):
        parts = [float(v) for v in value.replace(",", " ").split()]
        while len(parts) < 3:
            parts.append(0.0)
        return parts[:3]
    return [0.0, 0.0, 0.0]


def extract_entries(raw):
    if isinstance(raw, list):
        return raw
    if isinstance(raw, dict):
        for key in ARRAY_WRAPPER_KEYS:
            if key in raw and isinstance(raw[key], list):
                return raw[key]
        # some exports nest per-object-group, e.g. {"group": {"Objects":[...]}}
        for value in raw.values():
            if isinstance(value, dict):
                found = extract_entries(value)
                if found:
                    return found
    return []


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("input", help="Raw export from COT / VPP Admin Tools / other placement tool")
    parser.add_argument("output", help="Where to write the canonical SM_DungeonMod JSON")
    parser.add_argument("--classname-key", default=None, help="Override the classname field name")
    parser.add_argument("--position-key", default=None, help="Override the position field name")
    parser.add_argument("--orientation-key", default=None, help="Override the orientation field name")
    args = parser.parse_args()

    with open(args.input, "r", encoding="utf-8-sig") as f:
        raw = json.load(f)

    entries = extract_entries(raw)
    if not entries:
        print("Could not find a list of objects in the input file.", file=sys.stderr)
        print("If your export nests objects unusually, open an issue with a sample.", file=sys.stderr)
        sys.exit(1)

    result = []
    skipped = 0
    for entry in entries:
        if not isinstance(entry, dict):
            skipped += 1
            continue

        classname_key = find_key(entry, CLASSNAME_KEYS, args.classname_key)
        if not classname_key:
            skipped += 1
            continue

        position_key = find_key(entry, POSITION_KEYS, args.position_key)
        orientation_key = find_key(entry, ORIENTATION_KEYS, args.orientation_key)

        result.append({
            "ClassName": str(entry[classname_key]),
            "Position": to_vector(entry.get(position_key)) if position_key else [0.0, 0.0, 0.0],
            "Orientation": to_vector(entry.get(orientation_key)) if orientation_key else [0.0, 0.0, 0.0],
        })

    with open(args.output, "w", encoding="utf-8") as f:
        json.dump({"Objects": result}, f, ensure_ascii=False, indent=2)

    print(f"Wrote {len(result)} object(s) to {args.output}" + (f" ({skipped} skipped)" if skipped else ""))


if __name__ == "__main__":
    main()
