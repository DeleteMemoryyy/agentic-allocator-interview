#!/bin/sh
set -eu

task_root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
export PYTHONPATH="$task_root/workspace"

python3 -m compileall -q "$task_root/unlock_stage.py" "$task_root/workspace/jobrunner" "$task_root/workspace/tests"
python3 -m unittest discover -s "$task_root/workspace/tests" -p 'test*.py' -v
