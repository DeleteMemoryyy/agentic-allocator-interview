#!/usr/bin/env python3
"""创建与传输方式无关的提交快照。"""

from __future__ import annotations

import json
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUTPUT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT / "submission.zip"
EXCLUDED_PARTS = {".git", "__pycache__", ".pytest_cache", "build", "dist"}
HANDOFF_HEADINGS = (
    "## 交付结论",
    "## 已接受合同",
    "## 关键决策",
    "## 变更",
    "## 证据",
    "## 剩余风险",
    "## 回滚",
)


def validate() -> None:
    metadata = json.loads((ROOT / "submission.json").read_text())
    if metadata.get("schemaVersion") != 2 or metadata.get("taskId") != "durable-dispatch-protocol-v3":
        raise SystemExit("submission.json 的 schemaVersion 或 taskId 不受支持")
    if metadata.get("assignmentId") in {None, "", "public-template"}:
        raise SystemExit("缺少本场 assignmentId；请先应用面试官提供的 session overlay")
    tooling = metadata.get("tooling", {})
    if (
        not isinstance(tooling, dict)
        or tooling.get("primary") in {None, "", "fill-before-submit"}
        or tooling.get("model") in {None, "", "fill-before-submit"}
        or not isinstance(tooling.get("subagentsUsed"), bool)
        or type(tooling.get("maxConcurrentAgents")) is not int
        or tooling["maxConcurrentAgents"] < 1
    ):
        raise SystemExit("请先按本场实际情况填写 submission.json.tooling")
    if not (ROOT / "HANDOFF.md").is_file() or not (ROOT / "workspace").is_dir():
        raise SystemExit("必须包含 HANDOFF.md 和 workspace/")
    handoff = (ROOT / "HANDOFF.md").read_text(encoding="utf-8")
    missing = [heading for heading in HANDOFF_HEADINGS if heading not in handoff]
    if missing:
        raise SystemExit("HANDOFF.md 缺少标题：" + "、".join(missing))
    if "待填写" in handoff or "TODO" in handoff:
        raise SystemExit("HANDOFF.md 仍包含占位文本")
    if len(handoff.split()) > 800 or len(handoff) > 8000:
        raise SystemExit("HANDOFF.md 超过 800 个空白分词或 8,000 个字符")


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
