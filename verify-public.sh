#!/bin/sh
set -eu

workspace=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build_dir=$(mktemp -d "${TMPDIR:-/tmp}/allocator-public.XXXXXX")
trap 'rm -rf "$build_dir"' EXIT INT TERM
compiler=${CC:-clang}

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror \
  -fsanitize=address,undefined -I"$workspace" \
  "$workspace/allocator.c" "$workspace/heap_checker.c" "$workspace/runner.c" \
  -o "$build_dir/runner"

for trace in "$workspace"/public-tests/*.trace; do
  ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 "$build_dir/runner" "$trace"
done

"$compiler" -std=c11 -O1 -g -Wall -Wextra -Werror \
  -fsanitize=address,undefined -I"$workspace" \
  "$workspace/heap_checker.c" "$workspace/checker_smoke.c" \
  -o "$build_dir/checker-smoke"
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 "$build_dir/checker-smoke"

printf '%s\n' 'PUBLIC VERIFICATION PASS'
