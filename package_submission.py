#!/usr/bin/env python3
"""Create a transport-neutral submission snapshot."""

from __future__ import annotations

import json
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUTPUT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT / "submission.zip"
EXCLUDED_PARTS = {".git", "__pycache__", ".pytest_cache", "build", "dist"}


def validate() -> None:
    metadata = json.loads((ROOT / "submission.json").read_text())
    if metadata.get("schemaVersion") != 1 or metadata.get("taskId") != "durable-dispatch-protocol-v2":
        raise SystemExit("submission.json has an unsupported schema or taskId")
    if not (ROOT / "HANDOFF.md").is_file() or not (ROOT / "workspace").is_dir():
        raise SystemExit("HANDOFF.md and workspace/ are required")


def included(path: Path) -> bool:
    relative = path.relative_to(ROOT)
    return path.is_file() and not any(part in EXCLUDED_PARTS for part in relative.parts)


def main() -> None:
    validate()
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(OUTPUT, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for name in ("HANDOFF.md", "submission.json"):
            archive.write(ROOT / name, f"submission/{name}")
        for path in sorted((ROOT / "workspace").rglob("*")):
            if included(path):
                archive.write(path, "submission/" + path.relative_to(ROOT).as_posix())
    print(OUTPUT)


if __name__ == "__main__":
    main()
